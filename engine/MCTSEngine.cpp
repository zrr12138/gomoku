//
// Created by zhengran on 2024/2/8.
//

#include "MCTSEngine.h"

#include <memory>
#include <stack>
#include "glog/logging.h"
#include "common/timeutility.h"
#include "common/defer.h"
#include <cmath>
#include <chrono>
#include <thread>
#include <cassert>
#include <random>
#include <fstream>
#include <algorithm>

namespace gomoku {
    MCTSEngine::MCTSEngine(double explore_c) : C(explore_c), root_n(0) {

    }

    bool MCTSEngine::StartSearch(const ChessBoardState &state, bool black_first) {
        stop_.store(false);
        LOG(INFO) << __func__ << " board: " << state.hash() << " black_first: " << black_first;
        //初始化根节点x
        root_n = 0;
        root_black = black_first;
        root_node_ = std::make_unique<Node>(black_first, this);
        root_board_ = state;
        threadPool.Init(1, std::bind(&MCTSEngine::LoopExpandTree, this));
        threadPool.Start();
        return true;
    }

    void MCTSEngine::LoopExpandTree() {
        LOG(WARNING) << "start loop expand tree";
        SearchCtx ctx;
        ctx.board = root_board_;
        while (!stop_.load()) {
            root_node_->ExpandTree(&ctx);
            root_n++;
        }
    }

    bool MCTSEngine::Stop() {
        stop_.store(true);
        auto start = common::TimeUtility::GetTimeofDayMs();
        threadPool.Stop();
        LOG(INFO) << "thread pool stop in "
                  << common::TimeUtility::GetTimeofDayMs() - start << " ms";
        return true;
    }

    bool MCTSEngine::Action(ChessMove move) {
        return false;
    }

    ChessMove MCTSEngine::GetResult() {
        while (!root_node_->all_expanded.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        auto extremum = root_node_->moves.begin()->second->GetWinRate(root_black);
        auto it = root_node_->moves.begin();
        ChessMove move = it->first;
        while (++it != root_node_->moves.end()) {
            auto temp = it->second->GetWinRate(root_black);
            if (temp > extremum) {
                extremum = temp;
                move = it->first;
            }
        }
        return move;
    }

    void MCTSEngine::DumpTree() {
        std::ofstream outputFile("tree.txt");
        outputFile << "root_n:" << root_n << std::endl;
        PrintNode(outputFile, root_node_.get(), ChessMove(), 0);
        outputFile.close();
    }

    void MCTSEngine::PrintNode(std::ostream &os, Node *node, ChessMove move, int deep) {
        os << "\n";
        for (int i = 0; i < deep; i++) {
            os << "\t";
        }
        os << move;
        os << " bv:" << node->GetValue(true) << " wv:" << node->GetValue(false) << " b_win rate:"
           << node->GetWinRate(true) << " w_win rate:" << node->GetWinRate(false) << " bwc:" << node->black_win_count
           << " wwc" << node->white_win_count << " n:" << node->n;
        for (auto &mv: node->moves) {
            PrintNode(os, mv.second.get(), mv.first, deep + 1);
        }
    }

    Node::Node(bool isBlack, MCTSEngine *engine) : is_black(isBlack), n(0), black_win_count(0), white_win_count(0),
                                                   engine_(engine), all_expanded(false) {

    }

    void Node::UpdateValue(BoardResult res) {
        std::unique_lock<std::mutex> guard(value_mutex_);
        n++;
        if (res == BoardResult::BLACK_WIN) {
            black_win_count++;
        } else if (res == BoardResult::WHITE_WIN) {
            white_win_count++;
        }
    }


    BoardResult Node::ExpandTree(SearchCtx *ctx) {
        if (ctx->board.End() != BoardResult::NOT_END) {
            return ctx->board.End();
        }
        if (all_expanded.load()) {
            std::unique_lock<std::mutex> guard(struct_mutex_);
            //TODO(zrr12138) read lock
            auto it1 = moves.begin();
            auto it2 = it1;
            it2++;
            assert(it1->second);
            assert(it2->second);
            auto val1 = it1->second->GetValue(is_black);
            auto val2 = it2->second->GetValue(is_black);
            if (val1 < val2) {
                auto temp = std::move(*it1);
                moves.erase(it1);
                bool inserted = false;
                auto it = moves.begin();
                it++; // element2 is judged
                for (; it != moves.end(); it++) {
                    auto val = it->second->GetValue(is_black);
                    if (val < val1) {
                        it--;
                        moves.insert(it, std::move(temp));
                        inserted = true;
                        break;
                    }
                }
                if (!inserted) {
                    moves.emplace_back(std::move(temp));
                }
            }
            guard.unlock();
            auto move = moves.begin();
            ctx->board.Move(move->first);
            auto res = move->second->ExpandTree(ctx);
            ctx->board.WithdrawMove(move->first);
            UpdateValue(res);
            return res;
        } else {
            std::unique_lock<std::mutex> guard(struct_mutex_);
            if (unexpanded_nodes.empty()) {
                ctx->board.GetMoves(is_black, &unexpanded_nodes);
            }
            moves.emplace_back(unexpanded_nodes.back(), std::make_unique<Node>(!is_black, engine_));
            unexpanded_nodes.pop_back();
            auto move = moves.rbegin();
            ctx->board.Move(move->first);
            auto res = move->second->Simulation(ctx);
            ctx->board.WithdrawMove(move->first);
            UpdateValue(res);
            if (unexpanded_nodes.empty()) {
                all_expanded.store(true);
                auto is_black_ = is_black;
                moves.sort([&is_black_](const std::pair<ChessMove, std::unique_ptr<Node>> &x,
                                        const std::pair<ChessMove, std::unique_ptr<Node>> &y) -> bool {
                    return x.second->GetValue(is_black_) > y.second->GetValue(is_black_);
                });
            }
            return res;
        }
    }

    BoardResult Node::Simulation(SearchCtx *ctx) {
        thread_local uint32_t coords[BOARD_SIZE * BOARD_SIZE];
        thread_local bool coords_inited = false;
        thread_local std::random_device rd;  // 随机数种子
        thread_local std::minstd_rand rng(rd());  // 随机数生成器
        if (!coords_inited) {
            for (uint32_t i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
                coords[i] = i;
            }
            coords_inited = true;
        }
        std::shuffle(coords, coords + BOARD_SIZE * BOARD_SIZE, rng);
        bool black_turn = is_black;
        auto board = ctx->board; //copy for not withdraw
        uint32_t index = 0;
        while (board.End() == BoardResult::NOT_END && index < BOARD_SIZE * BOARD_SIZE) {
            uint32_t x = coords[index] / BOARD_SIZE;
            uint32_t y = coords[index] % BOARD_SIZE;
            if (board.GetChessAt(x, y) != Chess::EMPTY) {
                index++;
                continue;
            }
            ChessMove move(black_turn, x, y);
            board.Move(move);
            black_turn = !black_turn;
            index++;
        }
        BoardResult end = board.End();
        UpdateValue(end);
        return end;
    }

    double Node::GetValue(bool black_value) {
        double dw, dn, total_n;
        {
            std::unique_lock<std::mutex> guard(value_mutex_);
            assert(n != 0);
            if (black_value) {
                dw = static_cast<double >(black_win_count);
            } else {
                dw = static_cast<double >(white_win_count);
            }
            dn = static_cast<double >(n);
        }
        total_n = static_cast<double >(engine_->root_n);
        auto engine = engine_;
        double res = dw / dn + engine->C * std::sqrt(std::log(total_n) / dn);
        return res;
    }

    double Node::GetWinRate(bool black_rate) {
        double dw, dn;
        {
            std::unique_lock<std::mutex> guard(value_mutex_);
            assert(n != 0);
            if (black_rate) {
                dw = static_cast<double >(black_win_count);
            } else {
                dw = static_cast<double >(white_win_count);
            }
            dn = static_cast<double >(n);
        }
        return dw / dn;
    }


}

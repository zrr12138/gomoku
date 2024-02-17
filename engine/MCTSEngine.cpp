//
// Created by zhengran on 2024/2/8.
//

#include "MCTSEngine.h"

#include <memory>
#include <stack>
#include "glog/logging.h"
#include "common/timeutility.h"
#include "common/defer.h"
#include "common/rw_lock.h"
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
        root_node_ = std::make_shared<Node>(black_first, this);
        root_board_ = state;
        threadPool.Init(8, std::bind(&MCTSEngine::LoopExpandTree, this));
        threadPool.Start();
        return true;
    }

    void MCTSEngine::LoopExpandTree() {
        LOG(WARNING) << "start loop expand tree";
        SearchCtx ctx;
        ctx.board = root_board_;
        while (!stop_.load()) {
            // LOG(INFO) << "start expand tree whit ctx addr:" << &ctx << " ctx.board.hash:" << ctx.board.hash();
            root_node_->ExpandTree(&ctx);
            root_n.fetch_add(1,std::memory_order_relaxed);
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
        common::ReadLockGuard r_guard(root_node_->moves_lock);
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
        os << " value:" << node->GetValue() << " b_win rate:"
           << node->GetWinRate(true) << " w_win rate:" << node->GetWinRate(false) << " bwc:" << node->black_win_count
           << " wwc" << node->white_win_count << " n:" << node->n;
        for (auto &mv: node->moves) {
            PrintNode(os, mv.second.get(), mv.first, deep + 1);
        }
    }

    Node::Node(bool isBlack, MCTSEngine *engine) : is_black(isBlack), n(0), black_win_count(0), white_win_count(0),
                                                   engine_(engine), access_cnt(0), unexpanded_nodes_inited(false) {

    }

    void Node::UpdateValue(BoardResult res) {
        n.fetch_add(1,std::memory_order_relaxed);
        if (res == BoardResult::BLACK_WIN) {
            black_win_count.fetch_add(1,std::memory_order_relaxed);
        } else if (res == BoardResult::WHITE_WIN) {
            white_win_count.fetch_add(1,std::memory_order_relaxed);
        }
    }

    void Node::InitUnexpandedNode(SearchCtx *ctx) {
        if (!unexpanded_nodes_inited) {
            std::unique_lock<std::mutex> guard(unexpanded_nodes_lock);
            if (!unexpanded_nodes_inited) {
                ctx->board.GetMoves(is_black, &unexpanded_nodes);
                unexpanded_nodes_inited = true;
            }
        }
    }


    BoardResult Node::ExpandTree(SearchCtx *ctx) {
        if (ctx->board.End() != BoardResult::NOT_END) {
            return ctx->board.End();
        }
        InitUnexpandedNode(ctx);
        int64_t index = access_cnt.fetch_add(1);
        if (index >= unexpanded_nodes.size()) {
            if (index % 64 == 0) {
                //for performance
                std::pair<ChessMove, std::shared_ptr<Node>> move;
                {
                    common::ReadLockGuard r_guard(moves_lock);
                    assert(!moves.empty());
                    move = *std::max_element(moves.begin(), moves.end(),
                                             [](const std::pair<ChessMove, std::shared_ptr<Node>> &x,
                                                const std::pair<ChessMove, std::shared_ptr<Node>> &y) -> bool {
                                                 return x.second->GetValue() < y.second->GetValue();
                                             });
                }
                {
                    common::WriteLockGuard w_gurad(best_move_lock_);
                    best_move_ = move;
                }
            }
            std::pair<ChessMove, std::shared_ptr<Node>> best_move;
            {
                common::ReadLockGuard r_guard(best_move_lock_);
                assert(best_move_.second != nullptr);
                best_move = best_move_;
            }
            ctx->board.Move(best_move.first);
            auto res = best_move.second->ExpandTree(ctx);
            ctx->board.WithdrawMove(best_move.first);
            UpdateValue(res);
            return res;
        } else {
            assert(index < unexpanded_nodes.size());
            auto move = std::make_pair(unexpanded_nodes[index], std::make_shared<Node>(!is_black, engine_));
            if (index == 0) {
                common::WriteLockGuard w_gurad(best_move_lock_);
                best_move_ = move;
            }
            {
                common::WriteLockGuard w_gurad(moves_lock);
                moves.emplace_back(move);
            }
            ctx->board.Move(move.first);
            auto res = move.second->Simulation(ctx);
            ctx->board.WithdrawMove(move.first);
            UpdateValue(res);
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

    double Node::GetValue() {
        double dw, dn, total_n;
        {
            if (!is_black) {
                dw = static_cast<double >(black_win_count.load(std::memory_order_relaxed));
            } else {
                dw = static_cast<double >(white_win_count.load(std::memory_order_relaxed));
            }
            dn = static_cast<double >(n.load(std::memory_order_relaxed));
            if (dn == 0) {
                return 0;
            }
        }
        total_n = static_cast<double >(engine_->root_n.load(std::memory_order_relaxed));
        return dw / dn + engine_->C * std::sqrt(std::log(total_n) / dn);
    }

    double Node::GetWinRate(bool black_rate) {
        double dw, dn;
        {
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

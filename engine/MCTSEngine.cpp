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
    MCTSEngine::MCTSEngine(uint32_t thread_num, double explore_c) : C(explore_c), root_n(0), thread_num_(thread_num) {

    }

    bool MCTSEngine::StartSearch(const ChessBoardState &state, bool black_first) {
        assert(thread_num_ > 512);
        stop_.store(false);
        LOG(INFO) << __func__ << " board: " << state.hash() << " black_first: " << black_first;
        //初始化根节点x
        root_n = 0;
        root_black = black_first;
        root_node_ = std::make_shared<Node>(black_first, this);
        root_board_ = state;
        threadPool.Init(thread_num_, std::bind(&MCTSEngine::LoopExpandTree, this));
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
        auto is_black = root_node_->is_black;
        auto index = std::max_element(root_node_->sub_nodes, root_node_->sub_nodes + root_node_->moves_size,
                                      [is_black](const std::shared_ptr<Node> &x,
                                                 const std::shared_ptr<Node> &y) -> bool {
                                          assert(x);
                                          assert(y);
                                          return x->GetWinRate(is_black) < y->GetWinRate(is_black);
                                      }) - root_node_->sub_nodes;

        return root_node_->moves[index];
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
        for (int i = 0; i < node->moves_size; i++) {
            PrintNode(os, node->sub_nodes[i].get(), node->moves[i], deep + 1);
        }
    }

    Node::Node(bool isBlack, MCTSEngine *engine) : is_black(isBlack), n(0), black_win_count(0), white_win_count(0),
                                                   engine_(engine), access_cnt(0), moves_size(0), best_move_index(-1) {

    }

    void Node::UpdateValue(BoardResult res) {
        n++;
        if (res == BoardResult::BLACK_WIN) {
            black_win_count++;
        } else if (res == BoardResult::WHITE_WIN) {
            white_win_count++;
        }
    }

    void Node::InitUnexpandedNode(const ChessBoardState &board) {
        assert(board.End() == BoardResult::NOT_END);
        for (uint32_t i = 0; i < BOARD_SIZE; i++) {
            for (uint32_t j = 0; j < BOARD_SIZE; j++) {
                if (board.GetChessAt(i, j) == Chess::EMPTY) {
                    moves[moves_size++] = ChessMove(is_black, i, j);
                }
            }
        }
    }


    BoardResult Node::ExpandTree(SearchCtx *ctx) {
        if (ctx->board.End() != BoardResult::NOT_END) {
            return ctx->board.End();
        }
        int64_t index = access_cnt.fetch_add(1);
        if (index == 0) {
            InitUnexpandedNode(ctx->board);
            assert(moves_size == BOARD_SIZE * BOARD_SIZE - ctx->board.GetMoveNums());
        }
        if (index >= (BOARD_SIZE * BOARD_SIZE - ctx->board.GetMoveNums())) {
            if (index % 64 == 0 && index - engine_->thread_num_ > 0) {
                assert(moves_size > 0);
                int64_t r = std::min(index - engine_->thread_num_, moves_size.load());
                best_move_index = std::max_element(sub_nodes, sub_nodes + r,
                                                   [](const std::shared_ptr<Node> &x,
                                                      const std::shared_ptr<Node> &y) -> bool {
                                                       assert(x);
                                                       assert(y);
                                                       return x->GetValue() < y->GetValue();
                                                   }) - sub_nodes;
            }
            auto move_index = best_move_index.load();
            assert(move_index > 0);
            auto move = moves[move_index];
            auto node = sub_nodes[move_index];
            ctx->board.Move(move);
            auto res = node->ExpandTree(ctx);
            ctx->board.WithdrawMove(move);
            UpdateValue(res);
            return res;
        } else {
            while (index >= moves_size);
            auto move = moves[index];
            auto node = std::make_shared<Node>(!is_black, engine_);
            sub_nodes[index] = node;
            if (index == 1) {
                best_move_index = 1;
            }
            ctx->board.Move(move);
            auto res = node->Simulation(ctx);
            ctx->board.WithdrawMove(move);
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
                dw = static_cast<double >(black_win_count);
            } else {
                dw = static_cast<double >(white_win_count);
            }
            dn = static_cast<double >(n);
            if (dn == 0) {
                return 0;
            }
        }
        total_n = static_cast<double >(engine_->root_n);
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

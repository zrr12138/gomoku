//
// Created by zhengran on 2024/2/8.
//

#include "MCTSEngine.h"
#include <iostream>
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
    MCTSEngine::MCTSEngine(int thread_num, double explore_c) : C(explore_c), thread_num_(thread_num) {

    }

    bool MCTSEngine::StartSearch(const ChessBoardState &state, bool black_first) {
        std::cout << "thread_num_: " << thread_num_ << std::endl;
        assert(thread_num_ < 512);
        stop_.store(false);
        LOG(INFO) << __func__ << " board: " << state.hash() << " black_first: " << black_first;
        //初始化根节点x
        root_node_ = std::make_shared<Node>(black_first, this);
        root_board_ = std::make_shared<ChessBoardState>(state);
        threadPool.Init(thread_num_, std::bind(&MCTSEngine::LoopExpandTree, this));
        threadPool.Start();
        return true;
    }

    void MCTSEngine::LoopExpandTree() {
        LOG(WARNING) << "start loop expand tree";
        while (!stop_.load()) {
            std::shared_ptr<Node> root_node;
            SearchCtx ctx;
            {
                common::ReadLockGuard gurad(root_lock_);
                ctx.board = *root_board_;
                root_node = root_node_;
            }
            root_node->ExpandTree(&ctx);
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
        std::shared_ptr<Node> node;
        {
            common::ReadLockGuard gurad1(root_node_->move2node_lock_);
            for (auto &move_node: root_node_->move2node_) {
                if (move == move_node.first) {
                    node = move_node.second;
                    break;
                }
            }
        }
        if (node == nullptr) {
            node = std::make_shared<Node>(!root_node_->is_black, this);
        }
        {
            common::WriteLockGuard guard(root_lock_);
            assert(root_board_->Move(move));
            root_node_ = node;
        }
        if (root_board_->End() != BoardResult::NOT_END) {
            Stop();
        }
        return true;
    }

    ChessMove MCTSEngine::GetResult() {
        bool is_black;
        {
            common::ReadLockGuard guard(root_lock_);
            is_black = root_node_->is_black;
        }
        auto move = std::max_element(root_node_->move2node_.begin(), root_node_->move2node_.end(),
                                     [is_black](const std::pair<ChessMove, std::shared_ptr<Node>> &x,
                                                const std::pair<ChessMove, std::shared_ptr<Node>> &y) -> bool {
                                         return x.second->GetWinRate(is_black) < y.second->GetWinRate(is_black);
                                     })->first;
        return move;
    }

    void MCTSEngine::DumpTree() {
        std::ofstream outputFile("tree.txt");
        outputFile << "root_n:" << root_node_->n << std::endl;
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
        for (auto &move_node: node->move2node_) {
            PrintNode(os, move_node.second.get(), move_node.first, deep + 1);
        }
    }

    int64_t MCTSEngine::GetRootN() {
        return root_node_->n;
    }

    Node::Node(bool isBlack, MCTSEngine *engine) : is_black(isBlack), n(0), black_win_count(0), white_win_count(0),
                                                   engine_(engine), access_cnt(0), inited(false),
                                                   unexpanded_move_size(0), unexpanded_moves(nullptr) {

    }

    void Node::UpdateValue(BoardResult res) {
        n++;
        if (res == BoardResult::BLACK_WIN) {
            black_win_count++;
        } else if (res == BoardResult::WHITE_WIN) {
            white_win_count++;
        }
    }

    BoardResult Node::ExpandTree(SearchCtx *ctx) {
        if (ctx->board.End() != BoardResult::NOT_END) {
            UpdateValue(ctx->board.End());
            return ctx->board.End();
        }
        auto max_move_size = BOARD_SIZE * BOARD_SIZE - ctx->board.GetMoveNums();
        int64_t index = access_cnt.fetch_add(1);
        //init
        if (index == 0) {
            unexpanded_moves = new ChessMove[max_move_size];
            Init(ctx->board);
        }
        while (!inited.load(std::memory_order_relaxed));
        assert(unexpanded_move_size <= max_move_size);
        if (index >= unexpanded_move_size) {
            std::pair<ChessMove, std::shared_ptr<Node>> move_node;
            {
                common::ReadLockGuard gurad(best_move_lock_);
                move_node = best_move_node_;
            }
            if (move_node.second == nullptr && index < engine_->thread_num_) {
                bool fisrt_loop = true;
                while (true) {
                    if (!fisrt_loop)
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    common::ReadLockGuard gurad1(move2node_lock_);
                    if (!move2node_.empty()) break;
                    fisrt_loop = false;
                }
            }
            if (index % 64 == 0 || move_node.second == nullptr) {
                common::ReadLockGuard gurad1(move2node_lock_);
                auto best_move = *std::max_element(move2node_.begin(), move2node_.end(),
                                                   [](const std::pair<ChessMove, std::shared_ptr<Node>> &x,
                                                      const std::pair<ChessMove, std::shared_ptr<Node>> &y) -> bool {
                                                       return x.second->GetValue() < y.second->GetValue();
                                                   });
                common::WriteLockGuard gurad2(best_move_lock_);
                best_move_node_ = best_move;
                move_node = best_move;
            }
            assert(move_node.second);
            auto &move = move_node.first;
            auto &node = move_node.second;
            assert(move.x != -1 && move.y != -1 && node);
            ctx->board.Move(move);
            auto res = node->ExpandTree(ctx);
            UpdateValue(res);
            return res;
        } else {
            auto move = unexpanded_moves[index];
            std::pair<ChessMove, std::shared_ptr<Node>> move_node =
                    std::make_pair(move, std::make_shared<Node>(!is_black, engine_));
            auto &node = move_node.second;
            assert(move.x != -1 && move.y != -1 && node);
            {
                common::WriteLockGuard gurad(move2node_lock_);
                move2node_.push_back(move_node);
            }
            ctx->board.Move(move);
            auto res = node->Simulation(ctx);
            UpdateValue(res);
            return res;
        }
    }

    BoardResult Node::Simulation(SearchCtx *ctx) {
        thread_local int coords[BOARD_SIZE * BOARD_SIZE];
        thread_local bool coords_inited = false;
        thread_local std::random_device rd;  // 随机数种子
        thread_local std::minstd_rand rng(rd());  // 随机数生成器
        if (!coords_inited) {
            for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
                coords[i] = i;
            }
            coords_inited = true;
        }
        std::shuffle(coords, coords + BOARD_SIZE * BOARD_SIZE, rng);
        bool black_turn = is_black;
        auto &board = ctx->board;
        int index = 0;
        while (board.End() == BoardResult::NOT_END && index < BOARD_SIZE * BOARD_SIZE) {
            int x = coords[index] / BOARD_SIZE;
            int y = coords[index] % BOARD_SIZE;
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
        total_n = static_cast<double >(engine_->root_node_->n);
        return dw / dn + engine_->C * std::sqrt(std::log(total_n) / dn);
    }

    double Node::GetWinRate(bool black_rate) {
        double dw, dn;
        {
            if (black_rate) {
                dw = static_cast<double >(black_win_count);
            } else {
                dw = static_cast<double >(white_win_count);
            }
            dn = static_cast<double >(n);
            if (dn == 0) {
                return 0;
            }
        }
        return dw / dn;
    }

    BoardResult Node::Simulation2(SearchCtx *ctx) {
        thread_local int coords[BOARD_SIZE * BOARD_SIZE];
        thread_local bool coords_inited = false;
        thread_local std::random_device rd;  // 随机数种子
        thread_local std::minstd_rand rng(rd());  // 随机数生成器
        if (!coords_inited) {
            for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
                coords[i] = i;
            }
            coords_inited = true;
        }
        std::shuffle(coords, coords + BOARD_SIZE * BOARD_SIZE, rng);
        bool vis[BOARD_SIZE * BOARD_SIZE] = {false};
        bool black_turn = is_black;
        auto &board = ctx->board;
        int index = 0;
        while (board.End() == BoardResult::NOT_END && board.GetMoveNums() < BOARD_SIZE * BOARD_SIZE) {
            assert(index < BOARD_SIZE * BOARD_SIZE);
            if (vis[index]) {
                index = (index + 1) % (BOARD_SIZE * BOARD_SIZE);
                continue;
            }
            int x = coords[index] / BOARD_SIZE;
            int y = coords[index] % BOARD_SIZE;
            if (board.GetChessAt(x, y) != Chess::EMPTY) {
                index = (index + 1) % (BOARD_SIZE * BOARD_SIZE);
                vis[index] = true;
                continue;
            }
            ChessMove move(black_turn, x, y);
            if (!board.IsCutMove(move)) {
                board.Move(move);
                black_turn = !black_turn;
                vis[index] = true;
            }
            index = (index + 1) % (BOARD_SIZE * BOARD_SIZE);
        }
        BoardResult end = board.End();
        UpdateValue(end);
        return end;
    }

    Node::~Node() {
        if (unexpanded_moves) {
            delete[] unexpanded_moves;
        }
    }

    void Node::Init(const ChessBoardState &board) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board.GetChessAt(i, j) == Chess::EMPTY && !board.IsCutMove({is_black, i, j})) {
                    unexpanded_moves[unexpanded_move_size++] = {is_black, i, j};
                }
            }
        }
        if (board.GetMoveNums() == 0) {
            unexpanded_moves[unexpanded_move_size++] = {is_black, 7, 7};
        }
        inited.store(true, std::memory_order_relaxed);
    }


}

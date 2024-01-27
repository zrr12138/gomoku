//
// Created by zrr on 2024/1/5.
//

#include "Engine.h"
#include <vector>
#include <utility>
#include <mutex>
#include <memory>
#include <map>
#include <cassert>
#include <chrono>
#include <thread>
#include <set>
#include <algorithm>
#include <cmath>
#include "glog/logging.h"

namespace gomoku {

    std::pair<ChessMove, int64_t>
    Engine::DFS(Engine::SearchCtx *ctx, bool is_max, int64_t upper_bound, int64_t lower_bound) {
        ctx->search_node++;
        if (ctx->current_depth >= ctx->depth_limit || ctx->board.IsEnd() || stop_.load()) {
            ctx->leaf_node++;
            ChessMove _{};
            return std::make_pair(_, evaluate_(ctx->board));
        }
        std::vector<ChessMove> moves;
        ctx->board.GetMoves(is_max, &moves);
        std::pair<ChessMove, int64_t> result;

        if (is_max) { result.second = INT64_MIN; }
        else { result.second = INT64_MAX; }
        for (auto &move: moves) {
            assert(ctx->board.Move(move));
            ctx->moves_.push_back(move);
            ctx->current_depth++;
            std::pair<ChessMove, int64_t> node_result;
            if (is_max) {
                node_result = DFS(ctx, false, INT64_MAX, result.second);
                node_result.first = move;
            } else {
                node_result = DFS(ctx, true, result.second, INT64_MIN);
                node_result.first = move;
            }
            ctx->current_depth--;
            ctx->moves_.pop_back();
            assert(ctx->board.WithdrawMove(move));
            if (is_max) {
                if (node_result.second > result.second) {
                    result = node_result;
                }
            } else {
                if (node_result.second < result.second) {
                    result = node_result;
                }
            }
            if ((result.second >= upper_bound && upper_bound != INT64_MAX) ||
                (result.second <= lower_bound && lower_bound != INT64_MIN)) {
                return result;
            }
        }
        return result;
    }

    void Engine::StartSearchInternal(const ChessBoardState &state, bool black_first) {
        LOG(INFO) << __func__ << "board:" << state.hash() << "black_first:" << black_first;
        depth2res_.clear();
        stop_.store(false);
        uint64_t depth = 1;
        while (!stop_.load()) {
            auto ctx = std::make_unique<Engine::SearchCtx>();
            ctx->board = state;
            ctx->current_depth = 0;
            ctx->depth_limit = depth;
            ctx->search_node = 0;
            ctx->leaf_node = 0;
            LOG(INFO) << "start dfs with board:" << ctx->board.hash() << " depth_limit:" << ctx->depth_limit;
            auto res = DFS(ctx.get(), black_first, INT64_MAX, INT64_MIN);
            LOG(INFO) << "get result of dfs board: " << ctx->board.hash() << " depth_limit:" << ctx->depth_limit
                      << " move:" << res.first << " score:" << res.second << " search node:" << ctx->search_node
                      << "leaf node:" << ctx->leaf_node;
            if (!stop_.load()) { //防止最后一次搜索是被打断的
                std::unique_lock<std::mutex> guard(map_mutex_);
                depth2res_[depth] = res;
            }
            depth++;
        }
    }

    ChessMove Engine::GetResult() {
        while (depth2res_.empty()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::unique_lock<std::mutex> guard(map_mutex_);
//        for(auto res:depth2res_){
//
//        }
        auto result = depth2res_.rbegin()->second.first;
        LOG(INFO) << __func__ << " get result:" << result;
        return result;
    }

    bool Engine::StartSearch(const ChessBoardState &state, bool black_first) {
        if (evaluate_ == nullptr) {
            LOG(ERROR) << "evaluate function is not register";
            return false;
        }
        LOG(INFO) << __func__ << " board: " << state.hash() << "black_first:" << black_first;
        Stop();
        stop_.store(false);
        if (state.IsEnd()) {
            return false;
        }
        taskThreadPool.Start(8, 3);
        taskThreadPool.Enqueue(std::bind(&Engine::StartSearchInternal, this, state, black_first));
        return true;
    }


    bool Engine::Stop() {
        stop_.store(true);
        auto start = std::chrono::high_resolution_clock::now();
        taskThreadPool.Stop();
        auto end = std::chrono::high_resolution_clock::now();
        LOG(INFO) << "task thread pool stop in "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms";
        return true;
    }

    Engine::Engine() : evaluate_(nullptr) {

    }

    void Engine::SetEvaluateFunction(std::function<int64_t(const ChessBoardState &)> fun) {
        evaluate_ = fun;
    }

    int64_t Engine::Evaluate(const ChessBoardState &board) {
        assert(evaluate_!= nullptr);
        return evaluate_(board);
    }

} // gomoku
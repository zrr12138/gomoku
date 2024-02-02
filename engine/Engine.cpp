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
#include "common/timeutility.h"
#include "common/defer.h"

namespace gomoku {

    Engine::SearchReturnCtx
    Engine::DFS(Engine::SearchCtx *ctx, bool is_max, int64_t upper_bound, int64_t lower_bound) {
        ctx->search_node++;
        auto hash = ctx->board.hash();
        MinMaxNode this_node = std::make_pair(hash, is_max);
        if (ctx->current_depth >= ctx->depth_limit || ctx->board.IsEnd() || stop_.load()) {
            ctx->leaf_node++;
            auto score = evaluate_(ctx->board);
            Engine::SearchReturnCtx result{ChessMove(), score, 0};
            return result;
        }
        // 记忆化上一次搜索的子节点情况，按照上一次搜索结果进行排序，优先搜索好的局面，利于剪支
        std::vector<ChessMove> moves;
        std::vector<std::pair<ChessMove, SearchReturnCtx>> *last_sub_nodes = nullptr;
        if (searched_sub_nodes_.find(this_node) != searched_sub_nodes_.end()) {
            last_sub_nodes = searched_sub_nodes_[this_node];
            ctx->hit_searched_sub_node_cnt_++;
        }
        if (last_sub_nodes != nullptr) {
            std::sort(last_sub_nodes->begin(), last_sub_nodes->end(),
                      [is_max](const std::pair<ChessMove, SearchReturnCtx> &x,
                               const std::pair<ChessMove, SearchReturnCtx> &y) -> bool {
                          if (is_max) {
                              return x.second.score_ > y.second.score_;
                          } else {
                              return x.second.score_ < y.second.score_;
                          }
                      });
            for (auto &it: *last_sub_nodes) {
                moves.emplace_back(it.first);
            }
        } else {
            std::vector<ChessMove> temp;
            ctx->board.GetMoves(is_max, &temp);
            for (auto &move: temp) {
                if (IsCutMove(ctx, move)) {
                    continue;
                }
                moves.emplace_back(move);
            }
        }

        Engine::SearchReturnCtx result;
        if (is_max) { result.score_ = INT64_MIN; }
        else { result.score_ = INT64_MAX; }
        auto current_sub_nodes = new std::vector<std::pair<ChessMove, SearchReturnCtx>>();
        defer({
                  delete last_sub_nodes;
                  searched_sub_nodes_[this_node] = current_sub_nodes;
              });
        for (auto it = moves.begin(); it != moves.end(); it++) {
            auto move = *it;
            assert(ctx->board.Move(move));
            ctx->moves_.push_back(move);
            ctx->current_depth++;
            Engine::SearchReturnCtx node_result;
            if (is_max) {
                node_result = DFS(ctx, false, INT64_MAX, result.score_);
                node_result.move_ = move;
            } else {
                node_result = DFS(ctx, true, result.score_, INT64_MIN);
                node_result.move_ = move;
            }
            ctx->current_depth--;
            ctx->moves_.pop_back();
            assert(ctx->board.WithdrawMove(move));
            if (is_max) {
                if (node_result.score_ > result.score_) {
                    result = node_result;
                }
            } else {
                if (node_result.score_ < result.score_) {
                    result = node_result;
                }
            }
            current_sub_nodes->emplace_back(move, node_result);
            if ((result.score_ >= upper_bound && upper_bound != INT64_MAX) ||
                (result.score_ <= lower_bound && lower_bound != INT64_MIN)) {
                it++;
                while (it != moves.end()) {
                    current_sub_nodes->emplace_back(*it, SearchReturnCtx{move, 0, 0});
                    it++;
                }
                return result;
            }
        }
        result.search_depth_ = std::max(ctx->depth_limit - ctx->current_depth, result.search_depth_ + 1);
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
            ctx->start_search_timestamp_ms = common::TimeUtility::GetTimeofDayMs();
            ctx->hit_searched_node_cnt_ = 0;
            LOG(INFO) << "start dfs with board: " << ctx->board.hash() << " depth_limit: " << ctx->depth_limit
                      << " sub_nodes_size:" << searched_sub_nodes_.size()
                      << " searched_node_size: " << searched_nodes_.size();
            auto res = DFS(ctx.get(), black_first, INT64_MAX, INT64_MIN);
            LOG(INFO) << "get result of dfs  depth_limit:" << ctx->depth_limit
                      << " move:" << res.move_ << " score:" << res.score_ << " max_search_depth:" << res.search_depth_
                      << " search node:" << ctx->search_node
                      << " leaf node:" << ctx->leaf_node << " hit_searched_sub_node:" << ctx->hit_searched_sub_node_cnt_
                      << " hit_searched_node:" << ctx->hit_searched_node_cnt_
                      << " cost:"
                      << common::TimeUtility::GetTimeofDayMs() - ctx->start_search_timestamp_ms << " ms"
                      << " is interrupt: " << stop_.load() << " board: " << ctx->board.hash();
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
        auto result = depth2res_.rbegin()->second;
        LOG(INFO) << __func__ << " get result:" << result.ToString();
        return result.move_;
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

    Engine::Engine() : evaluate_(nullptr), around({{1,  0},
                                                   {-1, 0},
                                                   {0,  1},
                                                   {0,  -1},
                                                   {1,  1},
                                                   {-1, -1},
                                                   {1,  -1},
                                                   {-1, 1}}) {

    }

    void Engine::SetEvaluateFunction(std::function<int64_t(const ChessBoardState &)> fun) {
        evaluate_ = fun;
    }

    int64_t Engine::Evaluate(const ChessBoardState &board) {
        assert(evaluate_ != nullptr);
        return evaluate_(board);
    }

    bool Engine::IsCutMove(const Engine::SearchCtx *ctx, const ChessMove &move) const {
        //如果当前点半径为2的范围内没有棋子，则直接剪掉
        for (auto &pos: around) {
            if (move.x + pos.first < BOARD_SIZE && move.y + pos.second < BOARD_SIZE &&
                ctx->board.GetChessAt(move.x + pos.first, move.y + pos.second) != Chess::EMPTY) {
                return false;
            }
            if (move.x + pos.first * 2 < BOARD_SIZE && move.y + pos.second * 2 < BOARD_SIZE &&
                ctx->board.GetChessAt(move.x + pos.first * 2, move.y + pos.second * 2) != Chess::EMPTY) {
                return false;
            }
        }
        if (ctx->board.isInit()) {
            return false;
        } else {
            return true;
        }
    }

    Engine::SearchReturnCtx::SearchReturnCtx(const ChessMove &move, int64_t score, int64_t searchDepth) : search_depth_(
            searchDepth), score_(score), move_(move) {}

    std::string Engine::SearchReturnCtx::ToString() const {
        std::stringstream ss;
        ss << "search_depth_: " << search_depth_ << " score_: " << score_ << " move_: " << move_;
        return ss.str();
    }

} // gomoku
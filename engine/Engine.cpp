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

namespace gomoku {

    std::pair<ChessMove, int64_t>
    Engine::DFS(Engine::SearchCtx *ctx, bool is_max, int64_t upper_bound, int64_t lower_bound) {
        if (ctx->current_depth >= ctx->depth_limit || ctx->board.IsEnd() || stop_.load()) {
            ChessMove _{};
            return std::make_pair(_, Evaluate(ctx->board));
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
            } else {
                node_result = DFS(ctx, true, result.second, INT64_MIN);
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
        depth2res_.clear();
        stop_.store(false);
        uint64_t depth = 1;
        while (!stop_.load()) {
            auto ctx = std::make_unique<Engine::SearchCtx>();
            ctx->board = state;
            ctx->current_depth = 0;
            ctx->depth_limit = depth;
            auto res = DFS(ctx.get(), black_first, INT64_MAX, INT64_MIN);
            if (!stop_.load()) { //防止最后一次搜索是被打断的
                std::unique_lock<std::mutex> guard(map_mutex_);
                depth2res_[depth] = res;
            }
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
        return depth2res_.rbegin()->second.first;
    }

    bool Engine::StartSearch(const ChessBoardState &state, bool black_first) {
        stop_.store(true);
        taskThreadPool.Stop();
        stop_.store(false);
        if (state.IsEnd()) {
            return false;
        }
        taskThreadPool.Enqueue(std::bind(&Engine::StartSearchInternal, this, state, black_first));
        return true;
    }

    int64_t Engine::Evaluate(const ChessBoardState &board) {
        if (board.IsEnd() == 1) {
            return INT64_MAX;
        }
        if (board.IsEnd() == -1) {
            return INT64_MIN;
        }
        int64_t result = 0;
        std::map<std::pair<uint32_t, uint32_t>, Chess> pos2chess;
        board.GetPositionMap(&pos2chess);
        std::vector<std::pair<int, int>> dir = {{1,  0},
                                                {-1, 0},
                                                {0,  1},
                                                {0,  -1},
                                                {1,  1},
                                                {-1, -1},
                                                {1,  -1},
                                                {-1, 1}};
        //std::set<std::pair<int, int>> vis;
        for (auto &it: pos2chess) {
            /*
             * 以当前棋子为中心计算得分
             * 对于一个棋子，在一个给定方向上，遇到对方棋子或者边界前一定呈现类似"111001"的排列,1代表棋子，0代表空格。
             * 对这样的序列进行计分即可
             * */
            int64_t score = 0;
            int x = static_cast<int>(it.first.first), y = static_cast<int>(it.first.second);
//            if(vis.find(std::make_pair(x,y))!=vis.end()){
//                continue;
//            }
            std::vector<std::vector<int>> dir2seq;//存放序列
            for (int k = 0; k < 8; k++) {
                int i = x, j = y;
                auto delta = dir[k];
                std::vector<int>seq;
                while (true) {
                    int next_i = i + delta.first;
                    int next_j = j + delta.second;
                    //遇到边界
                    if (next_i < 0 || next_i >= BOARD_SIZE ||
                        next_j < 0 || next_j >= BOARD_SIZE) {
                        break;
                    }
                    if (board.GetChessAt(next_i, next_j) == it.second) {
                        seq.emplace_back(1);
//                        vis.emplace(std::make_pair(next_i,next_j));
                    } else if (board.GetChessAt(next_i, next_j) == EMPTY) {
                        seq.emplace_back(0);
                    } else {//遇到对方棋子
                        break;
                    }
                    i = next_i;
                    j = next_j;
                }
                dir2seq.emplace_back(std::move(seq));
            }
            auto EvaluateSeq=[](const std::vector<int> &seq)->int64_t{
                if(seq.size()<5){
                    return 0;
                }
                int64_t res=0;
                int64_t temp=0;
                for(auto it:seq){
                    if(it){
                        temp++;
                    }
                    else{
                        temp=std::max(temp-1,static_cast<int64_t>(0));
                    }
                    res+=temp;
                }
                return res;
            };
            for(int k=0;k<8;k+=2){
                std::vector<int>seq;
                seq.insert(seq.end(),dir2seq[k].rbegin(),dir2seq[k].rend());
                seq.emplace_back(1);
                seq.insert(seq.end(),dir2seq[k+1].begin(),dir2seq[k+1].end());
                score+=EvaluateSeq(seq);
            }
            if(it.second==WHITE){
                score=-score;
            }
            result+=score;
        }
        return result;
    }

} // gomoku
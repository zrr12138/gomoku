//
// Created by zrr on 2024/1/5.
//

#ifndef GOMOKU_ENGINE_H
#define GOMOKU_ENGINE_H

#include "ChessBoardState.h"
#include <utility>
#include <mutex>
#include <memory>
#include <map>
#include <vector>
#include "common/uncopyable.h"
#include <atomic>
#include "common/task_thread_pool.h"
namespace gomoku {

    class Engine {

    public:
        bool StartSearch(const ChessBoardState &state, bool black_first);//非阻塞,指定先手和局面开始搜索，中断上一次的搜索
        // json GetSearchTree(uint32_t depth); //指定深度打印搜索树信息
        ChessMove GetResult(); //获取搜索结果,该函数不应该中断搜索，可以反复调用获取最新的搜索结果
//        uint64_t GetSearchDepth();
        int64_t Evaluate(const ChessBoardState &board);
        int64_t CsyEvaluate(const ChessBoardState &board);
        bool Stop();
    private:
        struct SearchCtx{
            ChessBoardState board;
            std::vector<ChessMove> moves_;
            uint32_t current_depth;
            uint32_t depth_limit;
            uint64_t search_node;
            uint64_t leaf_node;
        };
        common::TaskThreadPool<> taskThreadPool;
        std::mutex map_mutex_;//保护下面两个数据结构
        std::map<uint64_t ,std::pair<ChessMove,int64_t> >depth2res_;
        std::atomic<bool> stop_;

        //TODO(zrr12138) 设置为std::function，可以动态设置评估函数
        /**
         * 局面评估函数
         * @param board 局面
         * @return 返回评估值，正数则为黑棋优势，负数为白棋优势，黑棋胜利为INT64_MAX，白棋胜利为INT64_MIN
         */

        void StartSearchInternal(const ChessBoardState &state,bool black_first);
        /**
         * 内部的搜索函数，超出范围说明当前路径可以被剪枝，将已经计算出的结果返回给上层
         * @param ctx 搜索上下文，局面，历史移动信息，当前深度，最大深度等
         * @param is_max true 取后继状态的最大值,false 取后继状态的最小值
         * @param upper_bound 分值上限
         * lower_bound 分值下限
         * @return <最佳走法,分值>
         */
        std::pair<ChessMove, int64_t> DFS(SearchCtx *ctx,bool is_max,int64_t upper_bound,int64_t lower_bound);
    };

} // gomoku

#endif //GOMOKU_ENGINE_H

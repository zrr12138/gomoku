//
// Created by zhengran on 2024/2/8.
//
#include <queue>
#include "ChessBoardState.h"
#include "common/task_thread_pool.h"
#include "common/thread_pool.h"
#include <cmath>
#include <list>
#include "common/rw_lock.h"

#ifndef GOMOKU_MCTSENGINE_H
#define GOMOKU_MCTSENGINE_H

namespace gomoku {
    struct SearchCtx;

    class MCTSEngine;

    struct Node {
        Node(bool isBlack, MCTSEngine *engine);

        std::atomic<int64_t> n, black_win_count, white_win_count;

        std::vector<ChessMove> unexpanded_nodes;
        std::mutex unexpanded_nodes_lock;
        std::atomic<bool> unexpanded_nodes_inited;
        std::atomic<int64_t> access_cnt;

        std::list<std::pair<ChessMove, std::shared_ptr<Node>>> moves;
        common::RWLock moves_lock;

        std::pair<ChessMove, std::shared_ptr<Node>> best_move_;
        common::RWLock best_move_lock_;
        bool is_black;
        MCTSEngine *engine_;

        void UpdateValue(BoardResult res);

        double GetValue();

        double GetWinRate(bool black_rate);

        BoardResult ExpandTree(SearchCtx *ctx);//需要确保最后能还原ctx中的内容用于下一次搜索
        BoardResult Simulation(SearchCtx *ctx); //黑棋赢则返回1否则返回0
        inline void InitUnexpandedNode(SearchCtx *ctx);
    };

    struct SearchCtx {
        ChessBoardState board;
        std::vector<ChessMove> moves_;
    };

    class MCTSEngine {
        friend Node;
    public:
        explicit MCTSEngine(double explore_c = std::sqrt(2));

        bool StartSearch(const ChessBoardState &state, bool black_first);

        bool Action(ChessMove move);

        ChessMove GetResult(); //获取搜索结果,该函数不应该中断搜索，可以反复调用获取最新的搜索结果
        bool Stop();

        void DumpTree();

    private:
        std::atomic<int64_t> root_n;
        const double C;
        std::atomic<bool> stop_;
        common::ThreadPool threadPool;
        std::shared_ptr<Node> root_node_;
        bool root_black;
        ChessBoardState root_board_;

        void LoopExpandTree();

        void PrintNode(std::ostream &os, Node *node, ChessMove move, int deep);
    };
};

#endif //GOMOKU_MCTSENGINE_H

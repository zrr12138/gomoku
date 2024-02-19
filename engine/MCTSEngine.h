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

        std::atomic<int64_t> access_cnt;

        ChessMove moves[BOARD_SIZE*BOARD_SIZE];
        std::shared_ptr<Node> sub_nodes[BOARD_SIZE*BOARD_SIZE];


        std::atomic<int64_t> moves_size;
        std::atomic<int64_t> best_move_index;

        bool is_black;
        MCTSEngine *engine_;

        void UpdateValue(BoardResult res);

        double GetValue();

        double GetWinRate(bool black_rate);

        BoardResult ExpandTree(SearchCtx *ctx);//需要确保最后能还原ctx中的内容用于下一次搜索
        BoardResult Simulation(SearchCtx *ctx); //黑棋赢则返回1否则返回0
        inline void InitUnexpandedNode(const ChessBoardState &board);
    };

    struct SearchCtx {
        ChessBoardState board;
        std::vector<ChessMove> moves_;
    };

    class MCTSEngine {
        friend Node;
    public:
        explicit MCTSEngine(uint32_t thread_num, double explore_c = std::sqrt(2));

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
        uint32_t thread_num_;

        void LoopExpandTree();

        void PrintNode(std::ostream &os, Node *node, ChessMove move, int deep);
    };
};

#endif //GOMOKU_MCTSENGINE_H

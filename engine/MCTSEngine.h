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
        ~Node();
        std::atomic<int64_t> n, black_win_count, white_win_count;

        std::atomic<int64_t> access_cnt;
        std::atomic<bool> inited;
        ChessMove *unexpanded_moves;
        int unexpanded_move_size;

        std::list<std::pair<ChessMove, std::shared_ptr<Node>>> move2node_;
        common::RWLock move2node_lock_;
        std::pair<ChessMove, std::shared_ptr<Node>> best_move_node_;
        common::RWLock best_move_lock_;

        bool is_black;
        MCTSEngine *engine_;

        void UpdateValue(BoardResult res);

        double GetValue();

        double GetWinRate(bool black_rate);

        BoardResult ExpandTree(SearchCtx *ctx);//需要确保最后能还原ctx中的内容用于下一次搜索
        BoardResult Simulation(SearchCtx *ctx); //黑棋赢则返回1否则返回0
        BoardResult Simulation2(SearchCtx *ctx);
        void Init(const ChessBoardState &borad);
    };

    struct SearchCtx {
        ChessBoardState board;
        std::vector<ChessMove> moves_;
    };

    class MCTSEngine {
        friend Node;
    public:
        explicit MCTSEngine(int thread_num, double explore_c = std::sqrt(2));

        bool StartSearch(const ChessBoardState &state, bool black_first);

        bool Action(ChessMove move);

        ChessMove GetResult(); //获取搜索结果,该函数不应该中断搜索，可以反复调用获取最新的搜索结果
        bool Stop();

        void DumpTree();

        int64_t GetRootN();

        void LogPath();

    private:
        const double C;
        std::atomic<bool> stop_;
        common::ThreadPool threadPool;
        common::RWLock root_lock_;
        std::shared_ptr<Node> root_node_;
        std::shared_ptr<ChessBoardState> root_board_;
        int thread_num_;

        void LoopExpandTree();

        void PrintNode(std::ostream &os, Node *node, ChessMove move, int deep);

        void LogPathNode(std::stringstream &line, Node *node);

    };
};

#endif //GOMOKU_MCTSENGINE_H

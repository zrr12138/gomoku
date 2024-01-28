//
// Created by zhengran on 2024/1/27.
//
#include "ChessBoardState.h"

#ifndef GOMOKU_EVALUATE_H
#define GOMOKU_EVALUATE_H
#define BLACK_WIN (1LL<<60)
#define BLACK_LOSS -(BLACK_WIN)
namespace gomoku {
    class Evalute{
    public:
        static int64_t evaluate_1(const ChessBoardState &board);
        static int64_t evaluate_2(const ChessBoardState &board);
        static int64_t evaluate_3(const ChessBoardState &board);
    };

}


#endif //GOMOKU_EVALUATE_H

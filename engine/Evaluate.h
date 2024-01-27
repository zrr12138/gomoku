//
// Created by zhengran on 2024/1/27.
//
#include "ChessBoardState.h"

#ifndef GOMOKU_EVALUATE_H
#define GOMOKU_EVALUATE_H
#define BLACK_WIN INT64_MAX
#define BLACK_LOSS INT64_MIN
namespace gomoku {
    class Evalute{
    public:
        static int64_t evaluate_1(const ChessBoardState &board);
        static int64_t evaluate_2(const ChessBoardState &board);
        static int64_t evaluate_3(const ChessBoardState &board);
    };

}


#endif //GOMOKU_EVALUATE_H

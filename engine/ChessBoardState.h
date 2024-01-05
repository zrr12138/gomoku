//
// Created by zrr on 2024/1/5.
//
#include <stdint.h>
#ifndef GOMOKU_CHESSBOARDSTATE_H
#define GOMOKU_CHESSBOARDSTATE_H
enum Chess{
    EMPTY=0,
    BLACK=1,
    WHITE=2
};
namespace gomoku {
    const uint32_t BOARD_SIZE=15;
    class ChessBoardState {
        Chess board[BOARD_SIZE][BOARD_SIZE];
        uint64_t hash(){
            uint64_t h=0;
            for(int i=0;i<BOARD_SIZE;i++){
                for(int j=0;j<BOARD_SIZE;j++){
                    h=h*3+static_cast<uint64_t>(board[i][j]);
                }
            }
            return h;
        }
    };

}

#endif //GOMOKU_CHESSBOARDSTATE_H

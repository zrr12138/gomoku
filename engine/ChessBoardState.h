//
// Created by zrr on 2024/1/5.
//
#include <stdint.h>
#include <vector>
#include <map>
#include <ostream>

#ifndef GOMOKU_CHESSBOARDSTATE_H
#define GOMOKU_CHESSBOARDSTATE_H
enum Chess {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};
namespace gomoku {
    const uint32_t BOARD_SIZE = 15;
    struct ChessMove {
        bool is_black;
        uint32_t x, y;

        friend std::ostream &operator<<(std::ostream &os, const ChessMove &move);

        ChessMove(bool isBlack, uint32_t x, uint32_t y);
        ChessMove();

    };

    class ChessBoardState {
    private:
        int is_end;
        Chess board[BOARD_SIZE][BOARD_SIZE]{};
        void update_is_end_from(uint32_t x,uint32_t y); //以某个点为中心判断游戏是否结束。
    public:
        ChessBoardState();

        friend std::ostream &operator<<(std::ostream &os, const ChessBoardState &state);

        uint64_t hash() const;

        void GetMoves(bool is_black, std::vector<ChessMove> *moves) const;

        Chess GetChessAt(uint32_t x, uint32_t y) const;

        bool ClearChessAt(uint32_t x, uint32_t y);

        bool Move(ChessMove move);

        /**
     * 判断游戏是否结束,O（1）复杂度
     * @return 0代表未结束，1代表黑棋获胜，0代表白棋获胜
     */
        int IsEnd() const;

        bool WithdrawMove(ChessMove move);

        void GetPositionVec(std::vector<std::pair<uint32_t, uint32_t>> *black_pos,
                            std::vector<std::pair<uint32_t, uint32_t>> *white_pos) const;
        void GetPositionMap(std::map<std::pair<uint32_t,uint32_t>,Chess> *pos2chess) const;
        void PrintOnTerminal();
    };

}

#endif //GOMOKU_CHESSBOARDSTATE_H

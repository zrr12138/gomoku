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
enum BoardResult {
    BLACK_WIN = 0,
    WHITE_WIN = 1,
    BALANCE = 2,
    NOT_END = 3,
};
namespace gomoku {
    const int BOARD_SIZE = 15;

    struct ChessMove {
        bool is_black;
        int x, y;

        bool operator==(const ChessMove &rhs) const;

        bool operator!=(const ChessMove &rhs) const;

        friend std::ostream &operator<<(std::ostream &os, const ChessMove &move);

        ChessMove(bool isBlack, int x, int y);

        ChessMove();

    };

    class ChessBoardState {
    private:
        int is_end;
        bool is_init;
        int move_num;
    public:
        bool isInit() const;

        void setIsInit(bool isInit);

    private:
        Chess board[BOARD_SIZE][BOARD_SIZE]{};

        void update_is_end_from(int x, int y); //以某个点为中心判断游戏是否结束。
    public:
        ChessBoardState();

        explicit ChessBoardState(const std::vector<ChessMove> &moves);

        friend std::ostream &operator<<(std::ostream &os, const ChessBoardState &state);

        uint64_t hash() const;

        void GetMoves(bool is_black, std::vector<ChessMove> *moves) const;

        Chess GetChessAt(int x, int y) const;

        bool Move(ChessMove move);

        int GetMoveNums() const;

        ChessMove GetNthMove(bool is_black, int index);

        /**
     * 判断游戏是否结束,O（1）复杂度
     * @return 0代表未结束，1代表黑棋获胜，0代表白棋获胜
     */
        int IsEnd() const;

        BoardResult End() const;

        bool IsEmpty();

        void ClearBoard();

        ChessMove getRandMove(bool is_black);

        bool WithdrawMove(ChessMove move);

        void GetPositionVec(std::vector<std::pair<int, int>> *black_pos,
                            std::vector<std::pair<int, int>> *white_pos) const;

        void GetPositionMap(std::map<std::pair<int, int>, Chess> *pos2chess) const;

        void PrintOnTerminal();

        bool IsCutMove(const ChessMove &move) const;
    };

}

#endif //GOMOKU_CHESSBOARDSTATE_H

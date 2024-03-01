//
// Created by zrr on 2024/1/5.
//

#include <cassert>
#include "ChessBoardState.h"
#include <iostream>
#include "glog/logging.h"
#include <random>

namespace gomoku {

    uint64_t ChessBoardState::hash() const {
        uint64_t h = 0;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                h = h * 3 + static_cast<uint64_t>(board[i][j]);
            }
        }
        return h;
    }

    Chess ChessBoardState::GetChessAt(int x, int y) const {
        assert(x < BOARD_SIZE);
        assert(y < BOARD_SIZE);
        return board[x][y];
    }

    ChessBoardState::ChessBoardState(const std::vector<ChessMove> &moves) : is_end(0), is_init(true) {
        ClearBoard();
        for (auto &move: moves) {
            assert(ChessBoardState::Move(move));
        }
    }

//    bool ChessBoardState::ClearChessAt(int x, int y) {
//        if (board[x][y] == EMPTY) {
//            return false;
//        }
//        board[x][y] = EMPTY;
//        //判断局面是否结束
//        is_end = 0;
//        for (int i = 0; i < BOARD_SIZE; i++) {
//            for (int j = 0; j < BOARD_SIZE; j++) {
//                if (board[i][j] != EMPTY) {
//                    update_is_end_from(i, j);
//                }
//            }
//        }
//        return true;
//    }

    bool ChessBoardState::Move(ChessMove move) {
        assert(move.x < BOARD_SIZE);
        assert(move.y < BOARD_SIZE);
        auto &chess = board[move.x][move.y];
        if (chess != EMPTY) {
            LOG(ERROR) << "move failed, move: " << move;
            return false;
        }
        is_init = false;
        assert(is_end == 0);
        chess = move.is_black ? BLACK : WHITE;
        update_is_end_from(move.x, move.y);
        move_num++;
        return true;
    }


    int ChessBoardState::IsEnd() const {
        return is_end;
    }

    bool ChessBoardState::IsEmpty() {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] != EMPTY)return false;
            }
        }
        return true;
    }

    void ChessBoardState::ClearBoard() {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        is_end = 0;
        is_init = true;
        move_num = 0;
    }

    ChessBoardState::ChessBoardState() : is_end(0), is_init(true), move_num(0) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
    }

    bool ChessBoardState::WithdrawMove(ChessMove move) {
        Chess chess = move.is_black ? BLACK : WHITE;
        if (board[move.x][move.y] != chess) {
            return false;
        }
        board[move.x][move.y] = EMPTY;
        if (is_end) {
            is_end = 0;
        }
        move_num--;
        return true;
    }

    void ChessBoardState::update_is_end_from(int x, int y) {
        assert(board[x][y] != EMPTY);
//        int dir[8][2] = {{1,  0},
//                         {-1, 0},
//                         {0,  1},
//                         {0,  -1},
//                         {1,  1},
//                         {-1, -1},
//                         {1,  -1},
//                         {-1, 1}};
        int cnt[8] = {0};
        const int board_size = static_cast<int>(BOARD_SIZE);
        //0
        {
            int i = x, j = y;
            int dx = 1, dy = 0;
            int max_step = board_size - i;
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[0]++;
                if (cnt[0] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
        //1
        {
            int i = x, j = y;
            int dx = -1, dy = 0;
            int max_step = i;
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[1]++;
                if (cnt[1] + cnt[0] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
        //2
        {
            int i = x, j = y;
            int dx = 0, dy = 1;
            int max_step = board_size - j;
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[2]++;
                if (cnt[2] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
        //3
        {
            int i = x, j = y;
            int dx = 0, dy = -1;
            int max_step = j;
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[3]++;
                if (cnt[3] + cnt[2] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
        //4
        {
            int i = x, j = y;
            int dx = 1, dy = 1;
            int max_step = std::min(board_size - i, board_size - j);
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[4]++;
                if (cnt[4] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
        //5
        {
            int i = x, j = y;
            int dx = -1, dy = -1;
            int max_step = std::min(i, j);
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[5]++;
                if (cnt[5] + cnt[4] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
        //6
        {
            int i = x, j = y;
            int dx = 1, dy = -1;
            int max_step = std::min(board_size - i, j);
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[6]++;
                if (cnt[6] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
        //7
        {
            int i = x, j = y;
            int dx = -1, dy = 1;
            int max_step = std::min(i, board_size - j);
            while (max_step-- > 0 &&
                   board[i + dx][j + dy] == board[x][y]) {
                i += dx;
                j += dy;
                cnt[7]++;
                if (cnt[7] + cnt[6] >= 4) {
                    is_end = board[x][y] == BLACK ? 1 : -1;
                    return;
                }
            }
        }
    }

    void ChessBoardState::GetMoves(bool is_black, std::vector<ChessMove> *moves) const {
        assert(is_end == 0);
        moves->reserve(BOARD_SIZE * BOARD_SIZE);
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == EMPTY) {
                    moves->emplace_back(is_black, i, j);
                }
            }
        }
    }

    void ChessBoardState::GetPositionVec(std::vector<std::pair<int, int>> *black_pos,
                                         std::vector<std::pair<int, int>> *white_pos) const {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == BLACK) {
                    black_pos->push_back(std::make_pair(i, j));
                } else if (board[i][j] == WHITE) {
                    white_pos->push_back(std::make_pair(i, j));
                }
            }
        }
    }

    void ChessBoardState::GetPositionMap(std::map<std::pair<int, int>, Chess> *pos2chess) const {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] != EMPTY) {
                    pos2chess->emplace(std::make_pair(i, j), board[i][j]);
                }
            }
        }
    }

    void ChessBoardState::PrintOnTerminal() {
        std::cout << *this;
    }

    std::ostream &operator<<(std::ostream &os, const ChessBoardState &state) {
        os << std::endl;
        os << "  ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            os << std::hex << i << " ";
        }
        os << std::endl;
        for (int i = 0; i < BOARD_SIZE; i++) {
            os << i << " ";
            for (int j = 0; j < BOARD_SIZE; j++) {
                char c;
                switch (state.board[i][j]) {
                    case EMPTY:
                        c = '*';
                        break;
                    case BLACK:
                        c = 'X';
                        break;
                    case WHITE:
                        c = 'O';
                        break;
                    default:
                        assert(0);
                }
                os << c << " ";
            }
            os << std::endl;
        }
        os << std::dec << std::endl;
        return os;
    }

    bool ChessBoardState::isInit() const {
        return is_init;
    }

    ChessMove ChessBoardState::getRandMove(bool is_black) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, BOARD_SIZE - 1);
        int try_num = BOARD_SIZE;
        while (try_num--) {
            int x = dist(gen);
            int y = dist(gen);
            if (GetChessAt(x, y) == Chess::EMPTY) {
                return {is_black, x, y};
            }
        }
        std::vector<ChessMove> moves;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (GetChessAt(i, j) == Chess::EMPTY) {
                    moves.emplace_back(is_black, i, j);
                }
            }
        }
        if (moves.empty()) {
            return ChessMove();
        }
        std::uniform_int_distribution<int> dist2(0, moves.size() - 1);
        return moves[dist2(gen)];
    }

    BoardResult ChessBoardState::End() const {
        switch (is_end) {
            case 0:
                return BoardResult::NOT_END;
            case 1:
                return BoardResult::BLACK_WIN;
            case -1:
                return BoardResult::WHITE_WIN;
            default:
                assert(0);
        }
    }

    int ChessBoardState::GetMoveNums() const{
        return move_num;
    }

    ChessMove ChessBoardState::GetNthMove(bool is_black, int index) {
        int cnt = 0;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (GetChessAt(i, j) == Chess::EMPTY) {
                    if (cnt++ == index) {
                        return {is_black, i, j};
                    }
                }
            }
        }
        assert(false);
    }

    bool ChessBoardState::IsCutMove(const ChessMove &move) const {
        const int &x = move.x;
        const int &y = move.y;

        // 上
        if (x - 1 >= 0 && board[x - 1][y] != Chess::EMPTY) return false;
        // 下
        if (x + 1 < BOARD_SIZE && board[x + 1][y] != Chess::EMPTY) return false;
        // 左
        if (y - 1 >= 0 && board[x][y - 1] != Chess::EMPTY) return false;
        // 右
        if (y + 1 < BOARD_SIZE && board[x][y + 1] != Chess::EMPTY) return false;
        // 左上
        if (x - 1 >= 0 && y - 1 >= 0 && board[x - 1][y - 1] != Chess::EMPTY) return false;
        // 右上
        if (x - 1 >= 0 && y + 1 < BOARD_SIZE && board[x - 1][y + 1] != Chess::EMPTY) return false;
        // 左下
        if (x + 1 < BOARD_SIZE && y - 1 >= 0 && board[x + 1][y - 1] != Chess::EMPTY) return false;
        // 右下
        if (x + 1 < BOARD_SIZE && y + 1 < BOARD_SIZE && board[x + 1][y + 1] != Chess::EMPTY) return false;

        // 上
        if (x - 2 >= 0 && board[x - 2][y] != Chess::EMPTY) return false;
        // 下
        if (x + 2 < BOARD_SIZE && board[x + 2][y] != Chess::EMPTY) return false;
        // 左
        if (y - 2 >= 0 && board[x][y - 2] != Chess::EMPTY) return false;
        // 右
        if (y + 2 < BOARD_SIZE && board[x][y + 2] != Chess::EMPTY) return false;
        // 左上
        if (x - 2 >= 0 && y - 2 >= 0 && board[x - 2][y - 2] != Chess::EMPTY) return false;
        // 右上
        if (x - 2 >= 0 && y + 2 < BOARD_SIZE && board[x - 2][y + 2] != Chess::EMPTY) return false;
        // 左下
        if (x + 2 < BOARD_SIZE && y - 2 >= 0 && board[x + 2][y - 2] != Chess::EMPTY) return false;
        // 右下
        if (x + 2 < BOARD_SIZE && y + 2 < BOARD_SIZE && board[x + 2][y + 2] != Chess::EMPTY) return false;

        return true;
    }

    ChessMove::ChessMove(bool isBlack, int x, int y) : is_black(isBlack), x(x), y(y) {
    }

    ChessMove::ChessMove() : is_black(true), x(-1), y(-1) {
    }

    std::ostream &operator<<(std::ostream &os, const ChessMove &move) {
        os << " (" << move.x << "," << move.y << ",";
        if (move.is_black) {
            os << "b";
        } else {
            os << "w";
        }
        os << ") ";
        return os;
    }

    bool ChessMove::operator==(const ChessMove &rhs) const {
        return is_black == rhs.is_black &&
               x == rhs.x &&
               y == rhs.y;
    }

    bool ChessMove::operator!=(const ChessMove &rhs) const {
        return !(rhs == *this);
    }

}
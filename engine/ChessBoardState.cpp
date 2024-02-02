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

    Chess ChessBoardState::GetChessAt(uint32_t x, uint32_t y) const {
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

//    bool ChessBoardState::ClearChessAt(uint32_t x, uint32_t y) {
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
    }

    ChessBoardState::ChessBoardState() : is_end(0), is_init(true) {
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
        return true;
    }

    void ChessBoardState::update_is_end_from(uint32_t x, uint32_t y) {
        assert(board[x][y] != EMPTY);

        static std::vector<std::pair<int, int>> dir = {{1,  0},
                                                       {-1, 0},
                                                       {0,  1},
                                                       {0,  -1},
                                                       {1,  1},
                                                       {-1, -1},
                                                       {1,  -1},
                                                       {-1, 1}};
        static int cnt[8];
        memset(cnt, 0, sizeof(cnt));
        for (int k = 0; k < 8; k++) {
            int i = static_cast<int>(x), j = static_cast<int>(y);
            auto delta = dir[k];
            while (0 <= i + delta.first && i + delta.first < BOARD_SIZE &&
                   0 <= j + delta.second && j + delta.second < BOARD_SIZE &&
                   board[i + delta.first][j + delta.second] == board[x][y]) {
                i += delta.first;
                j += delta.second;
                cnt[k]++;
                if (((k & 1) && cnt[k] + cnt[k - 1] >= 4) || cnt[k] >= 4) {
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

    void ChessBoardState::GetPositionVec(std::vector<std::pair<uint32_t, uint32_t>> *black_pos,
                                         std::vector<std::pair<uint32_t, uint32_t>> *white_pos) const {
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

    void ChessBoardState::GetPositionMap(std::map<std::pair<uint32_t, uint32_t>, Chess> *pos2chess) const {
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
        std::uniform_int_distribution<uint32_t> dist(0, BOARD_SIZE - 1);
        int try_num = BOARD_SIZE;
        while (try_num--) {
            uint32_t x = dist(gen);
            uint32_t y = dist(gen);
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
        std::uniform_int_distribution<uint32_t> dist2(0, moves.size() - 1);
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

    ChessMove::ChessMove(bool isBlack, uint32_t x, uint32_t y) : is_black(isBlack), x(x), y(y) {
    }

    ChessMove::ChessMove() : is_black(true), x(UINT32_MAX), y(UINT32_MAX) {
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

}
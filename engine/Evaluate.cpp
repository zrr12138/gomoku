//
// Created by zhengran on 2024/1/27.
//

#include "Evaluate.h"
#include "ChessBoardState.h"
#include <iostream>

namespace gomoku {

    int64_t Evalute::evaluate_1(const ChessBoardState &board) {
        if (board.IsEnd() == 1) {
            return BLACK_WIN;
        }
        if (board.IsEnd() == -1) {
            return BLACK_LOSS;
        }
        int64_t result = 0;
        std::map<std::pair<uint32_t, uint32_t>, Chess> pos2chess;
        board.GetPositionMap(&pos2chess);
        std::vector<std::pair<int, int>> dir = {{1,  0},
                                                {-1, 0},
                                                {0,  1},
                                                {0,  -1},
                                                {1,  1},
                                                {-1, -1},
                                                {1,  -1},
                                                {-1, 1}};
        //std::set<std::pair<int, int>> vis;
        for (auto &it: pos2chess) {
            /*
             * 以当前棋子为中心计算得分
             * 对于一个棋子，在一个给定方向上，遇到对方棋子或者边界前一定呈现类似"111001"的排列,1代表棋子，0代表空格。
             * 对这样的序列进行计分即可
             * */
            int64_t score = 0;
            int x = static_cast<int>(it.first.first), y = static_cast<int>(it.first.second);
//            if(vis.find(std::make_pair(x,y))!=vis.end()){
//                continue;
//            }
            std::vector<std::vector<int>> dir2seq;//存放序列
            for (int k = 0; k < 8; k++) {
                int i = x, j = y;
                auto delta = dir[k];
                std::vector<int> seq;
                while (true) {
                    int next_i = i + delta.first;
                    int next_j = j + delta.second;
                    //遇到边界
                    if (next_i < 0 || next_i >= BOARD_SIZE ||
                        next_j < 0 || next_j >= BOARD_SIZE) {
                        break;
                    }
                    if (board.GetChessAt(next_i, next_j) == it.second) {
                        seq.emplace_back(1);
//                        vis.emplace(std::make_pair(next_i,next_j));
                    } else if (board.GetChessAt(next_i, next_j) == EMPTY) {
                        seq.emplace_back(0);
                    } else {//遇到对方棋子
                        break;
                    }
                    i = next_i;
                    j = next_j;
                }
                dir2seq.emplace_back(std::move(seq));
            }
            auto EvaluateSeq = [](const std::vector<int> &seq) -> int64_t {
                if (seq.size() < 5) {
                    return 0;
                }
                int64_t res = 0;
                int64_t temp = 0;

                for (auto it: seq) {
                    if (it) {
                        temp++;
                    } else {
                        temp = std::max(temp - 1, static_cast<int64_t>(0));
                    }
                    res += temp;
                }
                return res;
            };
            for (int k = 0; k < 8; k += 2) {
                std::vector<int> seq;
                seq.insert(seq.end(), dir2seq[k].rbegin(), dir2seq[k].rend());
                seq.emplace_back(1);
                seq.insert(seq.end(), dir2seq[k + 1].begin(), dir2seq[k + 1].end());
                score += EvaluateSeq(seq);
            }
            if (it.second == WHITE) {
                score = -score;
            }
            result += score;
        }
        return result;
    }

    int64_t Evalute::evaluate_2(const ChessBoardState &board) {
        if (board.IsEnd() == 1) {
            return BLACK_WIN;
        }
        if (board.IsEnd() == -1) {
            return BLACK_LOSS;
        }
        int64_t result = 0;
        int64_t position_value[BOARD_SIZE][BOARD_SIZE]; // 棋子位置价值矩阵
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                position_value[i][j] = std::min((int) BOARD_SIZE - 1 - i,
                                                std::min((int) BOARD_SIZE - 1 - j, std::min(j - 0, i - 0)));
            }
        }

        std::map<std::pair<uint32_t, uint32_t>, Chess> pos2chess;
        board.GetPositionMap(&pos2chess);
        std::vector<std::pair<int, int>> dir = {{1,  0},
                                                {-1, 0},
                                                {0,  1},
                                                {0,  -1},
                                                {1,  1},
                                                {-1, -1},
                                                {1,  -1},
                                                {-1, 1}};
        //std::set<std::pair<int, int>> vis;
        for (auto &it: pos2chess) {
            /*
             * 以当前棋子为中心计算得分
             * 对于一个棋子，在一个给定方向上，遇到对方棋子或者边界前一定呈现类似"111001"的排列,1代表棋子，0代表空格。
             * 对这样的序列进行计分即可
             * */
            int64_t score = 0;
            int x = static_cast<int>(it.first.first), y = static_cast<int>(it.first.second);
            if (it.second == BLACK) {
                score += 1 << (int64_t) position_value[x, y];
            } else if (it.second == WHITE) {
                score -= 1 << (int64_t) position_value[x, y];
            }

//            if(vis.find(std::make_pair(x,y))!=vis.end()){
//                continue;
//            }
            std::vector<std::vector<int>> dir2seq;//存放序列
            for (int k = 0; k < 8; k++) {
                int i = x, j = y;
                auto delta = dir[k];
                std::vector<int> seq;
                while (true) {
                    int next_i = i + delta.first;
                    int next_j = j + delta.second;
                    //遇到边界
                    if (next_i < 0 || next_i >= BOARD_SIZE ||
                        next_j < 0 || next_j >= BOARD_SIZE) {
                        break;
                    }
                    if (board.GetChessAt(next_i, next_j) == BLACK) {
                        seq.emplace_back(1);
//                        vis.emplace(std::make_pair(next_i,next_j));
                    } else if (board.GetChessAt(next_i, next_j) == EMPTY) {
                        seq.emplace_back(0);
                    } else if (board.GetChessAt(next_i, next_j) == WHITE) {
                        seq.emplace_back(2);
                    }
                    i = next_i;
                    j = next_j;
                }
                dir2seq.emplace_back(std::move(seq));
            }

            auto EvaluateSeq = [](const std::vector<int> &seq) -> std::pair<int64_t, int64_t> {
                std::pair<int64_t, int64_t> res;
                res.first = 0;
                res.second = 0;

                int64_t Cheng_5 = 1 << 30;
                int64_t Huo_4 = 1 << 25;
                int64_t Huo_3 = 1 << 20;
                int64_t Chong_4 = 1 << 10;
                int64_t Mian_3 = 1 << 5;

                int now_chess = seq[0];
                int left_pos = 0;
                int right_pos = 0; // 左闭右开
                int cnt = 0;
                for (auto it: seq) {
                    if (it == now_chess) {
                        cnt++;
                        right_pos++;
                    }
                    if (it != now_chess || right_pos == BOARD_SIZE) {
                        int bias = right_pos - left_pos;
                        if (bias == 5) {
                            if (now_chess == 1)res.first += Cheng_5;
                            else if (now_chess == 2)res.second += Cheng_5;
                        } else if (bias == 4) {
                            if (left_pos != 0 && right_pos != BOARD_SIZE && seq[left_pos - 1] == 0 &&
                                seq[right_pos] == 0) {
                                if (now_chess == 1)res.first += Huo_4;
                                else if (now_chess == 2)res.second += Huo_4;
                            } else {
                                if (now_chess == 1)res.first += Chong_4;
                                else if (now_chess == 2)res.second += Chong_4;
                            }
                        } else if (bias == 3) {
                            if (left_pos != 0 && right_pos != BOARD_SIZE && seq[left_pos - 1] == 0 &&
                                seq[right_pos] == 0) {
                                if (now_chess == 1)res.first += Huo_3;
                                else if (now_chess == 2)res.second += Huo_3;
                            } else {
                                if (now_chess == 1)res.first += Mian_3;
                                else if (now_chess == 2)res.second += Mian_3;
                            }
                        }
                        now_chess = it;
                        left_pos = right_pos;
                        right_pos++;
                    }
                }
                return res;
            };

            for (int k = 0; k < 8; k += 2) {
                std::vector<int> seq;
                seq.insert(seq.end(), dir2seq[k].rbegin(), dir2seq[k].rend());
                if (board.GetChessAt(x, y) == BLACK) seq.emplace_back(1);
                else if (board.GetChessAt(x, y) == WHITE) seq.emplace_back(2);
                else if (board.GetChessAt(x, y) == EMPTY) seq.emplace_back(0);
                seq.insert(seq.end(), dir2seq[k + 1].begin(), dir2seq[k + 1].end());
                std::pair<int64_t, int64_t> ans = EvaluateSeq(seq);
                if (it.second == BLACK) {
                    score += ans.first;
                    score -= ans.second;

                } else if (it.second == WHITE) {
                    score += ans.second;
                    score -= ans.first;
                    score = -score;
                }
            }
            result += score;
        }
        return result;
    }

    int64_t Evalute::evaluate_3(const ChessBoardState &board) {
        if (board.IsEnd() == 1) {
            return BLACK_WIN;
        }
        if (board.IsEnd() == -1) {
            return BLACK_LOSS;
        }
        int64_t res = 0;
        int64_t e1 = 0, e2 = 0, x = 0;
        bool is_black;
        auto sub_evaluate = [&]() -> int64_t {
            //std::cout << e1 << " " << x << " " << e2 << std::endl;
            if (e1 + e2 + x < 5) {
                return 0;
            }
            int64_t ans = (15 * x + e1) * (15 * x + e2);
            if (!is_black) {
                ans = -ans;
            }
            return ans;
        };
        auto automaton_clear = [&]() -> void {
            res += sub_evaluate();
            e1 = 0;
            e2 = 0;
            x = 0;
        };
        auto automaton_move = [&](Chess chess) -> void {
            e1 = e2;
            e2 = 0;
            x = 1;
            is_black = (chess == Chess::BLACK);
        };
        auto automaton_push = [&](Chess chess) -> void {
            if (chess == EMPTY) {
                if (x == 0) {
                    e1++;
                } else {
                    e2++;
                }
                return;
            }
            if (x == 0) {
                is_black = (chess == Chess::BLACK);
                x++;
                return;
            }
            if (e2 == 0 && is_black == (chess == Chess::BLACK)) {
                x++;
                return;
            }
            res += sub_evaluate();
            automaton_move(chess);
        };
        //横
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                automaton_push(board.GetChessAt(i, j));
            }
            automaton_clear();
        }
        //竖
        for (int j = 0; j < BOARD_SIZE; j++) {
            for (int i = 0; i < BOARD_SIZE; i++) {
                automaton_push(board.GetChessAt(i, j));
            }
            automaton_clear();
        }
        // 正对角
        for (int s = 0; s < BOARD_SIZE; s++) {
            int i = 0;
            int j = s;
            while (i < BOARD_SIZE && j < BOARD_SIZE) {
                automaton_push(board.GetChessAt(i++, j++));
            }
            automaton_clear();
        }
        for (int s = 1; s < BOARD_SIZE; s++) {
            int i = s;
            int j = 0;
            while (i < BOARD_SIZE && j < BOARD_SIZE) {
                automaton_push(board.GetChessAt(i++, j++));
            }
            automaton_clear();
        }
        // 反对角
        for (int s = 0; s < BOARD_SIZE; s++) {
            int i = s;
            int j = 0;
            while (i >= 0 && i < BOARD_SIZE && j < BOARD_SIZE) {
                automaton_push(board.GetChessAt(i--, j++));
            }
            automaton_clear();
        }
        for (int s = 1; s < BOARD_SIZE; s++) {
            int i = BOARD_SIZE - 1;
            int j = s;
            while (i >= 0 && i < BOARD_SIZE && j < BOARD_SIZE) {
                automaton_push(board.GetChessAt(i--, j++));
            }
            automaton_clear();
        }
        return res;
    }

}
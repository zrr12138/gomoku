//
// Created by zrr on 2024/1/5.
//
#include <iostream>
#include "common/task_thread_pool.h"
#include "Engine.h"
#include "ChessBoardState.h"
#include "glog/logging.h"

void EvaluateManualTest();

void EngineManualTest();

void BoardEvaluateTest(std::vector<gomoku::ChessMove> &black_moves, std::vector<gomoku::ChessMove> &white_moves);
void ChessBoard_1();
void ChessBoard_2();
void ChessBoard_3();
void ChessBoard_4();
void ChessBoard_5();
void ChessBoard_6();
void ChessBoard_7();

#include <iostream>
#include <cmath>

int main(int argc, char *argv[]) {
    // Initialize Google’s logging library.
    google::InitGoogleLogging("gomoku");
    FLAGS_log_dir = ".";
    FLAGS_v = 2;
    EngineManualTest();
    //EvaluateManualTest();

//    ChessBoard_1();
//    ChessBoard_2();
//    ChessBoard_3();
//    ChessBoard_4();
//    ChessBoard_5();
//    ChessBoard_6();
//    ChessBoard_7();

}

void EngineManualTest() {
    gomoku::Engine engine;
    gomoku::ChessBoardState board;
    bool is_black = true;
    while (!board.IsEnd()) {
        board.PrintOnTerminal();
        std::cout << "当前局面评估分数为:" << engine.Evaluate(board) << std::endl;
        if (is_black) {
            std::cout << "请输入下一步棋的坐标:" << std::endl;
            int x, y;
            std::cin >> x >> y;
            LOG(INFO) << "user move x:" << x << "y:" << y;
            board.Move(gomoku::ChessMove(is_black, x, y));

        } else {
            if (!engine.StartSearch(board, false)) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(10));
            auto move = engine.GetResult();
            board.Move(move);
            std::cout << "engine move:" << move;
            engine.Stop();
        }
        is_black = !is_black;
    }
}

void EvaluateManualTest() {
    gomoku::Engine engine;
    gomoku::ChessBoardState board;
    bool is_black = true;
    uint32_t x = UINT32_MAX, y = UINT32_MAX;
    do {
        if (x != UINT32_MAX && y != UINT32_MAX) {
            board.Move(gomoku::ChessMove(is_black, x, y));
            is_black = !is_black;
        }
        board.PrintOnTerminal();
        std::cout << "当前局面评估分数为:" << engine.Evaluate(board) << std::endl;
        std::cout << "is_black:" << is_black << " 请输入下一步棋的坐标:" << std::endl;
    } while (std::cin >> x >> y);
}

void BoardEvaluateTest(std::vector<gomoku::ChessMove> &black_moves, std::vector<gomoku::ChessMove> &white_moves){
    gomoku::Engine engine;
    const std::pair<std::vector<gomoku::ChessMove>,std::vector<gomoku::ChessMove>> moves{black_moves, white_moves};
    gomoku::ChessBoardState board(moves);
    board.PrintOnTerminal();
    std::cout << "当前局面评估分数为:" << engine.Evaluate(board) << std::endl;
}

void ChessBoard_1(){
    std::vector<gomoku::ChessMove> black_moves = {{true,7,7},{true,7,8},{true,7,9}};
    std::vector<gomoku::ChessMove> white_moves = {{false,0,2},{false,0,3},{false,0,4}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_2(){
    std::vector<gomoku::ChessMove> black_moves = {{true,7,7},{true,7,8},{true,7,9}};
    std::vector<gomoku::ChessMove> white_moves = {{false,6,7},{false,6,8},{false,6,9}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_3(){
    std::vector<gomoku::ChessMove> black_moves = {{true,7,7},{true,7,6},{true,8,6},{true,9,5},{true,6,6}};
    std::vector<gomoku::ChessMove> white_moves = {{false,6,7},{false,7,8},{false,8,7},{false,6,8},{false,9,6}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_4(){
    std::vector<gomoku::ChessMove> black_moves = {{true,7,7},{true,7,6},{true,8,6},{true,9,5},{true,6,6}};
    std::vector<gomoku::ChessMove> white_moves = {{false,9,8},{false,7,8},{false,8,7},{false,6,8},{false,9,6}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_5(){
    std::vector<gomoku::ChessMove> black_moves = {{true,7,7},{true,7,8},{true,7,9}};
    std::vector<gomoku::ChessMove> white_moves = {{false,0,2},{false,0,3}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_6(){
    std::vector<gomoku::ChessMove> black_moves = {{true,7,7},{true,7,8},{true,7,9}};
    std::vector<gomoku::ChessMove> white_moves = {{false,6,7},{false,6,8}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_7(){
    std::vector<gomoku::ChessMove> black_moves = {{true,7,7},{true,7,8},{true,7,9},{true,7,10},{true,7,11}};
    std::vector<gomoku::ChessMove> white_moves = {{false,6,7},{false,6,8},{false,6,9},{false,6,10}};
    BoardEvaluateTest(black_moves, white_moves);
}
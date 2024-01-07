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

int main(int argc, char* argv[]) {
    // Initialize Google’s logging library.
    std::cout<<"hello world"<<std::endl;
    google::InitGoogleLogging("zrr11111");
    std::cout<<"hello world"<<std::endl;
    // ...
    LOG(INFO) << "Found " << 4 << " cookies";

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
            board.Move(gomoku::ChessMove(is_black, x, y));

        } else {
            engine.StartSearch(board, false);
            std::this_thread::sleep_for(std::chrono::seconds(3));
            auto move = engine.GetResult();
            board.Move(move);
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
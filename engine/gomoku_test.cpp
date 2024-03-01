//
// Created by zhengran on 2024/2/22.
//
//
// Created by zrr on 2024/1/5.
//
#include <iostream>
#include "common/task_thread_pool.h"
#include "Engine.h"
#include "ChessBoardState.h"
#include "glog/logging.h"
#include "Evaluate.h"
#include "MCTSEngine.h"
#include <cmath>
#include "gflags/gflags.h"
#include "common_flags.h"

void test1(gomoku::ChessBoardState *board, bool *black_first) {
    *black_first = true;
    board->Move(gomoku::ChessMove(true, 7, 7));
    board->Move(gomoku::ChessMove(true, 7, 6));
    board->Move(gomoku::ChessMove(true, 7, 8));
    board->Move(gomoku::ChessMove(true, 6, 7));
    board->Move(gomoku::ChessMove(true, 8, 8));
    board->Move(gomoku::ChessMove(true, 9, 8));
    board->Move(gomoku::ChessMove(true, 10, 10));
    board->Move(gomoku::ChessMove(true, 8, 9));
    board->Move(gomoku::ChessMove(true, 7, 10));
    board->Move(gomoku::ChessMove(true, 11, 8));

    board->Move(gomoku::ChessMove(false, 5, 6));
    board->Move(gomoku::ChessMove(false, 9, 9));
    board->Move(gomoku::ChessMove(false, 8, 7));
    board->Move(gomoku::ChessMove(false, 8, 5));
    board->Move(gomoku::ChessMove(false, 8, 4));
    board->Move(gomoku::ChessMove(false, 9, 6));
    board->Move(gomoku::ChessMove(false, 10, 7));
    board->Move(gomoku::ChessMove(false, 10, 8));
    board->Move(gomoku::ChessMove(false, 7, 9));
    board->Move(gomoku::ChessMove(false, 6, 8));
}

void test2(gomoku::ChessBoardState *board, bool *black_first) {
    *black_first = true;
    board->Move(gomoku::ChessMove(false, 4, 7));
    board->Move(gomoku::ChessMove(false, 4, 9));
    board->Move(gomoku::ChessMove(false, 5, 6));
    board->Move(gomoku::ChessMove(true, 5, 7));
    board->Move(gomoku::ChessMove(true, 5, 8));
    board->Move(gomoku::ChessMove(true, 5, 9));
    board->Move(gomoku::ChessMove(true, 6, 7));
    board->Move(gomoku::ChessMove(false, 6, 8));
    board->Move(gomoku::ChessMove(false, 6, 9));
    board->Move(gomoku::ChessMove(true, 7, 6));
    board->Move(gomoku::ChessMove(true, 7, 7));
    board->Move(gomoku::ChessMove(true, 7, 8));
    board->Move(gomoku::ChessMove(false, 7, 9));
    board->Move(gomoku::ChessMove(false, 8, 9));


}

void test3(gomoku::ChessBoardState *board, bool *black_first) {
    *black_first = true;
}

void Deduction(gomoku::ChessBoardState board, bool black) {
    board.PrintOnTerminal();
    gomoku::MCTSEngine engine(gomoku::FLAGS_thread_num);
    engine.StartSearch(board, black);
    int step = 1;
    while (board.End() == BoardResult::NOT_END) {
        std::this_thread::sleep_for(std::chrono::seconds(gomoku::FLAGS_think_time));
        auto move = engine.GetResult();
        std::cout << "engine move:" << move << std::endl;
        std::cout << "root_n:" << engine.GetRootN() << std::endl;
        board.Move(move);
        board.PrintOnTerminal();
        if (board.End() != BoardResult::NOT_END) {
            break;
        }
        engine.Action(move);
        black = !black;
        step++;
    }
    engine.Stop();
}

int main(int argc, char *argv[]) {
    // Initialize Google’s logging library.
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    google::InitGoogleLogging("gomoku");
    FLAGS_log_dir = ".";
    FLAGS_v = 2;
    gomoku::ChessBoardState board;
    bool black;
    test3(&board, &black);
    Deduction(board, black);
}

//
// Created by zrr on 2024/2/22.
//

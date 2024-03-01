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

int main(int argc, char *argv[]) {
    // Initialize Google’s logging library.
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    google::InitGoogleLogging("gomoku");
    FLAGS_log_dir = ".";
    FLAGS_v = 2;

    gomoku::MCTSEngine engine(gomoku::FLAGS_thread_num);
    gomoku::ChessBoardState board;
    board.Move(gomoku::ChessMove(true, 7, 7));
    board.Move(gomoku::ChessMove(true, 7, 8));
    board.Move(gomoku::ChessMove(true, 7, 9));
    board.Move(gomoku::ChessMove(false, 8, 7));
    board.Move(gomoku::ChessMove(false, 8, 8));
//    board.Move(gomoku::ChessMove(false,6,7));
//    board.Move(gomoku::ChessMove(true,7,8));
//    board.Move(gomoku::ChessMove(false,6,8));
//    board.Move(gomoku::ChessMove(true,7,9));
//    board.Move(gomoku::ChessMove(false,7,10));
//    board.Move(gomoku::ChessMove(true,7,6));
    engine.StartSearch(board, false);
    board.PrintOnTerminal();
    std::this_thread::sleep_for(std::chrono::seconds(gomoku::FLAGS_think_time));
    auto move = engine.GetResult();
    board.Move(move);
    board.PrintOnTerminal();
    std::cout << "engine move:" << move;
    engine.Stop();
    engine.DumpTree();
    std::cout << "root_n:" << engine.GetRootN();
}

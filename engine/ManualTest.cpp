//
// Created by zhengran on 2024/3/1.
//

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "MCTSEngine.h"
#include "ChessBoardState.h"
#include "common_flags.h"

DEFINE_bool(human_first, true, "");

void EngineManualTest() {
    gomoku::MCTSEngine engine(gomoku::FLAGS_thread_num);
    gomoku::ChessBoardState board;
    bool is_black = FLAGS_human_first;
    engine.StartSearch(board, is_black);
    while (!board.IsEnd()) {
        board.PrintOnTerminal();
        if (is_black) {
            std::cout << "Please enter the coordinates of the next move:" << std::endl;
            int x, y;
            std::cin >> x >> y;
            LOG(INFO) << "user move x:" << x << "y:" << y;
            board.Move(gomoku::ChessMove(is_black, x, y));
            engine.Action(gomoku::ChessMove(is_black, x, y));
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(gomoku::FLAGS_think_time));
            std::cout << "root_n:" << engine.GetRootN() << std::endl;
            auto move = engine.GetResult();
            board.Move(move);
            std::cout << "engine move:" << move;
            engine.Action(move);
        }
        is_black = !is_black;
    }
    engine.Stop();
}

int main(int argc, char *argv[]) {
    // Initialize Google’s logging library.
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    google::InitGoogleLogging("gomoku");
    FLAGS_log_dir = ".";
    FLAGS_v = 2;
    EngineManualTest();
}
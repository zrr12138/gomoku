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

void EvaluateManualTest();

void EngineManualTest();

void BoardEvaluateTest(std::vector<std::pair<int, int>> &black_moves,
                       std::vector<std::pair<int, int>> &white_moves);

void ChessBoard_1();

void ChessBoard_2();

void ChessBoard_3();

void ChessBoard_4();

void ChessBoard_5();

void ChessBoard_6();

void ChessBoard_7();

void ChessBoard_8();

void ChessBoard_9();

void ChessBoard_10();

void EngineTest_1();

void EngineTest_2();

void EngineTest_3();

void EngineTest_4();


int main(int argc, char *argv[]) {
    // Initialize Google’s logging library.
    google::InitGoogleLogging("gomoku");
    FLAGS_log_dir = ".";
    FLAGS_v = 2;
    EngineManualTest();
    //EvaluateManualTest();
    //EngineTest_1();
//    ChessBoard_1();
//    ChessBoard_2();
//    ChessBoard_3();
//    ChessBoard_4();
//    ChessBoard_5();
//    ChessBoard_6();
//    ChessBoard_7();
//    ChessBoard_8();
//    ChessBoard_9();
//    ChessBoard_10();
//    EngineTest_3();
    //EngineTest_4();
}

void EngineManualTest() {
    gomoku::MCTSEngine engine(64);
    gomoku::ChessBoardState board;
    bool is_black = false;
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
            std::this_thread::sleep_for(std::chrono::seconds(30));
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

void EvaluateManualTest() {
    gomoku::Engine engine;
    gomoku::ChessBoardState board;
    bool is_black = true;
    int x = -1, y = -1;
    do {
        if (x != -1 && y != -1) {
            board.Move(gomoku::ChessMove(is_black, x, y));
            is_black = !is_black;
        }
        board.PrintOnTerminal();
        std::cout << "The current situation evaluation score is:" << engine.Evaluate(board) << std::endl;
        std::cout << "is_black:" << is_black << " Please enter the coordinates of the next move:" << std::endl;
    } while (std::cin >> x >> y);
}

void BoardEvaluateTest(std::vector<std::pair<int, int>> &black_moves,
                       std::vector<std::pair<int, int>> &white_moves) {
    gomoku::Engine engine;
    engine.SetEvaluateFunction(&gomoku::Evalute::evaluate_3);
    std::vector<gomoku::ChessMove> moves;
    for (auto &move: black_moves) {
        moves.emplace_back(true, move.first, move.second);
    }
    for (auto &move: white_moves) {
        moves.emplace_back(false, move.first, move.second);
    }
    gomoku::ChessBoardState board(moves);
    board.PrintOnTerminal();
    std::cout << "The current situation evaluation score is:" << engine.Evaluate(board) << std::endl;
}

void ChessBoard_1() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 8},
                                                    {7, 9}};
    std::vector<std::pair<int, int>> white_moves = {{0, 2},
                                                    {0, 3},
                                                    {0, 4}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_2() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 8},
                                                    {7, 9}};
    std::vector<std::pair<int, int>> white_moves = {{6, 7},
                                                    {6, 8},
                                                    {6, 9}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_3() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 6},
                                                    {8, 6},
                                                    {9, 5},
                                                    {6, 6}};
    std::vector<std::pair<int, int>> white_moves = {{6, 7},
                                                    {7, 8},
                                                    {8, 7},
                                                    {6, 8},
                                                    {9, 6}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_4() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 6},
                                                    {8, 6},
                                                    {9, 5},
                                                    {6, 6}};
    std::vector<std::pair<int, int>> white_moves = {{9, 8},
                                                    {7, 8},
                                                    {8, 7},
                                                    {6, 8},
                                                    {9, 6}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_5() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 8},
                                                    {7, 9}};
    std::vector<std::pair<int, int>> white_moves = {{0, 2},
                                                    {0, 3}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_6() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 8},
                                                    {7, 9}};
    std::vector<std::pair<int, int>> white_moves = {{6, 7},
                                                    {6, 8}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_7() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 8},
                                                    {7, 9},
                                                    {7, 10},
                                                    {7, 11}};
    std::vector<std::pair<int, int>> white_moves = {};
    BoardEvaluateTest(black_moves, white_moves
    );
}

void ChessBoard_8() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7},
                                                    {7, 8},
                                                    {7, 9},
                                                    {7, 10},
    };
    std::vector<std::pair<int, int>> white_moves = {{7, 11},
                                                    {7, 6},
                                                    {8, 8},
                                                    {6, 8}};
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_9() {
    std::vector<std::pair<int, int>> black_moves = {{7, 7}};
    std::vector<std::pair<int, int>> white_moves = {{2, 3}};;
    BoardEvaluateTest(black_moves, white_moves);
}

void ChessBoard_10() {
    std::vector<std::pair<int, int>> black_moves = {{4, 3}};
    std::vector<std::pair<int, int>> white_moves = {{3, 4},
                                                    {2, 5}};
    BoardEvaluateTest(black_moves, white_moves);
}

void EngineTest_1() {
    gomoku::Engine engine;
    engine.SetEvaluateFunction(&gomoku::Evalute::evaluate_3);
    gomoku::ChessBoardState board;
    board.Move(gomoku::ChessMove(true, 7, 7));
    board.Move(gomoku::ChessMove(false, 5, 6));
    board.Move(gomoku::ChessMove(true, 6, 6));
    board.Move(gomoku::ChessMove(false, 6, 7));
    board.Move(gomoku::ChessMove(true, 5, 5));
    board.Move(gomoku::ChessMove(false, 7, 8));
    board.Move(gomoku::ChessMove(true, 8, 8));
    engine.StartSearch(board, false);
    board.PrintOnTerminal();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    auto move = engine.GetResult();
    board.Move(move);
    board.PrintOnTerminal();
    std::cout << "engine move:" << move;
    engine.Stop();
}

void EngineTest_2() {
    gomoku::Engine engine;
    engine.SetEvaluateFunction(&gomoku::Evalute::evaluate_3);
    gomoku::ChessBoardState board;
    board.Move(gomoku::ChessMove(true, 7, 7));
    board.Move(gomoku::ChessMove(false, 7, 8));
    board.Move(gomoku::ChessMove(true, 6, 7));
    board.Move(gomoku::ChessMove(false, 5, 7));
    board.Move(gomoku::ChessMove(true, 5, 8));
    engine.StartSearch(board, false);
    board.PrintOnTerminal();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    auto move = engine.GetResult();
    board.Move(move);
    board.PrintOnTerminal();
    std::cout << "engine move:" << move;
    engine.Stop();
}

void EngineTest_3() {
    gomoku::MCTSEngine engine(1);
    gomoku::ChessBoardState board;
    board.Move(gomoku::ChessMove(true, 7, 7));
    board.Move(gomoku::ChessMove(false, 7, 8));
    board.Move(gomoku::ChessMove(true, 6, 7));
    board.Move(gomoku::ChessMove(false, 5, 7));
    board.Move(gomoku::ChessMove(true, 5, 8));
    engine.StartSearch(board, false);
    board.PrintOnTerminal();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto move = engine.GetResult();
    board.Move(move);
    board.PrintOnTerminal();
    std::cout << "engine move:" << move;
    engine.Stop();
    engine.DumpTree();
}

void EngineTest_4() {
    gomoku::MCTSEngine engine(10);
    gomoku::ChessBoardState board;
    board.Move(gomoku::ChessMove(true, 7, 7));
    board.Move(gomoku::ChessMove(false, 6, 7));
    board.Move(gomoku::ChessMove(true, 7, 8));
    board.Move(gomoku::ChessMove(false, 6, 8));
    board.Move(gomoku::ChessMove(true, 7, 9));
    board.Move(gomoku::ChessMove(false, 7, 10));
    board.Move(gomoku::ChessMove(true, 7, 6));
    engine.StartSearch(board, false);
    board.PrintOnTerminal();
    std::this_thread::sleep_for(std::chrono::seconds(60));
    auto move = engine.GetResult();
    board.Move(move);
    board.PrintOnTerminal();
    std::cout << "engine move:" << move;
    engine.Stop();
    engine.DumpTree();
}

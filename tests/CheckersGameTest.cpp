#include <vector>
#include <gtest/gtest.h>
#include "../server/CheckersGame.h"

// Тест на проверку хода
TEST(CheckersGameTest, TestValidMove) {
    CheckersGame game;

    ASSERT_TRUE(game.makeMove(5, 0, 4, 1));
    ASSERT_EQ(game.board[5][0], '.');
    ASSERT_EQ(game.board[4][1], 'W');
}

// Тест на невозможность хода для пустой клетки
TEST(CheckersGameTest, TestInvalidMoveEmptyCell) {
    CheckersGame game;

    ASSERT_FALSE(game.makeMove(0, 0, 2, 2));
}

// Тест на захват фигуры
TEST(CheckersGameTest, TestCaptureMove) {
    CheckersGame game;

    game.makeMove(5, 0, 4, 1);
    game.makeMove(2, 3, 3, 2);

    ASSERT_TRUE(game.makeMove(4, 1, 2, 3));
    ASSERT_EQ(game.board[3][2], '.');
}

// Тест на проверку победителя
TEST(CheckersGameTest, TestWinner) {
    CheckersGame game;

    for (int i = 5; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            game.board[i][j] = '.';
        }
    }
    ASSERT_TRUE(game.checkWinner());
}

// Тест на невозможность хода за чужую фигуру
TEST(CheckersGameTest, TestInvalidMoveOpponentPiece) {
    CheckersGame game;
    ASSERT_FALSE(game.makeMove(1, 0, 2, 1));
}

// Тест на превращение в дамку
TEST(CheckersGameTest, TestPromotionToKing) {
    CheckersGame game;

    game.board[1][1] = 'W';
    game.board[0][0] = '.';

    ASSERT_TRUE(game.makeMove(1, 1, 0, 0));
    ASSERT_EQ(game.board[0][0], 'Q');
}

// Тест на движение дамки
TEST(CheckersGameTest, TestKingMove) {
    CheckersGame game;

    game.board[3][3] = 'Q';
    game.board[5][5] = '.';

    ASSERT_TRUE(game.makeMove(3, 3, 5, 5));
    ASSERT_EQ(game.board[5][5], 'Q');
    ASSERT_EQ(game.board[3][3], '.');
}

// Тест на дамочное взятие
TEST(CheckersGameTest, TestKingCapture) {
    CheckersGame game;

    game.board[5][5] = 'Q';
    game.board[4][4] = 'B';
    game.board[3][3] = '.';

    ASSERT_TRUE(game.makeMove(5, 5, 3, 3));
    ASSERT_EQ(game.board[3][3], 'Q');
}

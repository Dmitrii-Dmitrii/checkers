#ifndef CHECKERSGAME_H
#define CHECKERSGAME_H
#include <vector>
#include <iostream>

class CheckersGame {
public:
    CheckersGame();
    void resetBoard();
    void displayBoard();
    bool makeMove(int x1, int y1, int x2, int y2);
    char getCurrentPlayer() const;
    bool checkWinner();
    std::vector<std::vector<char>> board;
    const std::vector<std::vector<char>>& getBoard() const { return board; }
private:
    char currentPlayer;
    bool isValidMove(int x1, int y1, int x2, int y2);
    bool isCaptureMove(int x1, int y1, int x2, int y2);
    void promoteToKing(int x, int y);
    void removeCapturedPiece(int x1, int y1, int x2, int y2);
    void switchPlayer();
    bool hasMandatoryCapture();
    bool hasCaptureMove(int x, int y);
    char getKingSymbol(char player);
    bool isPathClear(int x1, int y1, int x2, int y2);
};

#endif // CHECKERSGAME_H
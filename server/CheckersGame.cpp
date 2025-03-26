#include "CheckersGame.h"

CheckersGame::CheckersGame() {
    resetBoard();
}

void CheckersGame::resetBoard() {
    board.assign(8, std::vector(8, '.'));
    currentPlayer = 'W';

    // for (int i = 0; i < 3; ++i) {
    //     for (int j = (i + 1) % 2; j < 8; j += 2) {
    //         board[i][j] = 'B';
    //     }
    // }
    for (int i = 5; i < 8; ++i) {
        for (int j = (i + 1) % 2; j < 8; j += 2) {
            board[i][j] = 'W';
        }
    }

    board[4][1] = 'B';
}

void CheckersGame::displayBoard() {
    std::cout << "  0 1 2 3 4 5 6 7" << std::endl;
    for (int i = 0; i < 8; ++i) {
        std::cout << i << " ";
        for (int j = 0; j < 8; ++j) {
            std::cout << board[i][j] << ' ';
        }
        std::cout << std::endl;
    }
}

bool CheckersGame::makeMove(int x1, int y1, int x2, int y2) {
    if (isValidMove(x1, y1, x2, y2)) {
        bool isCapture = isCaptureMove(x1, y1, x2, y2);

        if (isCapture) {
            removeCapturedPiece(x1, y1, x2, y2);
        }

        board[x2][y2] = board[x1][y1];
        board[x1][y1] = '.';

        promoteToKing(x2, y2);

        if (isCapture && hasCaptureMove(x2, y2)) {
            return true;
        }

        switchPlayer();
        return true;
    }

    return false;
}


bool CheckersGame::isValidMove(int x1, int y1, int x2, int y2) {
    if (x2 < 0 || x2 >= 8 || y2 < 0 || y2 >= 8 || board[x1][y1] == '.' || board[x2][y2] != '.') {
        return false;
    }
    char piece = board[x1][y1];
    if (piece != currentPlayer && piece != getKingSymbol(currentPlayer)) return false;

    int dx = x2 - x1;
    int dy = y2 - y1;

    if (hasMandatoryCapture() && !isCaptureMove(x1, y1, x2, y2)) {
        return false;
    }

    if (isCaptureMove(x1, y1, x2, y2)) {
        return true;
    }

    if (piece == 'B' && dx == 1 && std::abs(dy) == 1) return true;
    if (piece == 'W' && dx == -1 && std::abs(dy) == 1) return true;

    if (piece == 'K' || piece == 'Q') {
        if (std::abs(dx) == std::abs(dy) && isPathClear(x1, y1, x2, y2)) {
            return true;
        }
    }

    return false;
}


bool CheckersGame::isCaptureMove(int x1, int y1, int x2, int y2) {
    if (x2 < 0 || x2 >= 8 || y2 < 0 || y2 >= 8 || board[x2][y2] != '.') {
        return false;
    }

    int dx = x2 - x1;
    int dy = y2 - y1;
    char piece = board[x1][y1];

    if (std::abs(dx) != std::abs(dy)) return false;

    int midX, midY;
    if (piece == 'K' || piece == 'Q') {
        for (int i = 1; i < std::abs(dx); ++i) {
            midX = x1 + i * (dx / std::abs(dx));
            midY = y1 + i * (dy / std::abs(dy));
            if (board[midX][midY] != '.' && board[midX][midY] != currentPlayer && board[midX][midY] != getKingSymbol(currentPlayer)) {
                if (i == std::abs(dx) - 1) {
                    return true;
                }
                return false;
            }
        }
    } else if (std::abs(dx) == 2) {
        midX = (x1 + x2) / 2;
        midY = (y1 + y2) / 2;
        if (board[midX][midY] != '.' && board[midX][midY] != currentPlayer && board[midX][midY] != getKingSymbol(currentPlayer)) {
            return true;
        }
    }

    return false;
}


void CheckersGame::removeCapturedPiece(int x1, int y1, int x2, int y2) {
    int dx = (x2 > x1) ? 1 : -1;
    int dy = (y2 > y1) ? 1 : -1;

    int x = x1 + dx;
    int y = y1 + dy;

    while (x != x2 && y != y2) {
        if (board[x][y] != '.') {
            board[x][y] = '.';
            break;
        }
        x += dx;
        y += dy;
    }
}

void CheckersGame::promoteToKing(int x, int y) {
    if ((board[x][y] == 'B' && x == 7) || (board[x][y] == 'W' && x == 0)) {
        board[x][y] = (board[x][y] == 'B') ? 'K' : 'Q';
    }
}

void CheckersGame::switchPlayer() {
    currentPlayer = (currentPlayer == 'B') ? 'W' : 'B';
}

char CheckersGame::getCurrentPlayer() const {
    return currentPlayer;
}

bool CheckersGame::hasMandatoryCapture() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] == currentPlayer && hasCaptureMove(i, j)) {
                return true;
            }
        }
    }
    return false;
}

bool CheckersGame::hasCaptureMove(int x, int y) {
    char piece = board[x][y];
    if (piece != currentPlayer && piece != getKingSymbol(currentPlayer)) return false;

    if (piece == 'K' || piece == 'Q') {
        int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        for (auto& dir : directions) {
            int x1 = x + dir[0];
            int y1 = y + dir[1];
            while (x1 >= 0 && x1 < 8 && y1 >= 0 && y1 < 8) {
                int x2 = x1 + dir[0];
                int y2 = y1 + dir[1];
                if (x2 >= 0 && x2 < 8 && y2 >= 0 && y2 < 8 && isCaptureMove(x, y, x2, y2)) {
                    return true;
                }
                if (board[x1][y1] != '.') break;
                x1 += dir[0];
                y1 += dir[1];
            }
        }
    } else {
        int directions[4][2] = {{2, 2}, {2, -2}, {-2, 2}, {-2, -2}};
        for (auto& dir : directions) {
            int x2 = x + dir[0];
            int y2 = y + dir[1];
            if (isCaptureMove(x, y, x2, y2)) {
                return true;
            }
        }
    }
    return false;
}


char CheckersGame::getKingSymbol(char player) {
    return (player == 'B') ? 'K' : 'Q';
}

bool CheckersGame::isPathClear(int x1, int y1, int x2, int y2) {
    int dx = (x2 > x1) ? 1 : -1;
    int dy = (y2 > y1) ? 1 : -1;

    int x = x1 + dx;
    int y = y1 + dy;

    while (x != x2 && y != y2) {
        if (board[x][y] != '.') {
            return false;
        }
        x += dx;
        y += dy;
    }

    return true;
}

bool CheckersGame::checkWinner() {
    bool hasWhite = false, hasBlack = false;

    for (const auto& row : board) {
        for (char piece : row) {
            if (piece == 'W' || piece == 'Q') hasWhite = true;
            if (piece == 'B' || piece == 'K') hasBlack = true;
        }
    }

    if (!hasWhite) {
        std::cout << "Black is the winner!" << std::endl;
        return true;
    }
    if (!hasBlack) {
        std::cout << "White is the winner!" << std::endl;
        return true;
    }

    return false;
}
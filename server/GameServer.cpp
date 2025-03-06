#include "GameServer.h"
#include <iostream>
#include <cstdio>

GameServer::GameServer(std::shared_ptr<INetworkHandler> netHandler)
    : networkHandler(std::move(netHandler)), game() {}

void GameServer::run() {
    networkHandler->send("Welcome to Checkers! Type moves as: x1 y1 x2 y2");
    while (true) {
        game.displayBoard();
        std::cout << "Сейчас ходят " << (game.getCurrentPlayer() == 'B' ? "черные:" : "белые:") << std::endl;
        std::string command = networkHandler->receive();
        if (command == "exit") break;
        
        int x1, y1, x2, y2;
        if (parseMove(command, x1, y1, x2, y2)) {
            if (game.makeMove(x1, y1, x2, y2)) {
                networkHandler->send("Move accepted");
            } else {
                networkHandler->send("Invalid move");
            }
        } else {
            networkHandler->send("Invalid input format");
        }

        if (game.checkWinner()) break;
    }
}

bool GameServer::parseMove(const std::string& input, int& x1, int& y1, int& x2, int& y2) {
    return sscanf(input.c_str(), "%d %d %d %d", &x1, &y1, &x2, &y2) == 4;
}
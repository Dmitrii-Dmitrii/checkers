#include "GameServer.h"
#include <iostream>
#include <cstdio>
#include <thread>

GameServer::GameServer(std::shared_ptr<INetworkHandler> netHandler)
    : networkHandler(std::move(netHandler)), game(), running(false), returnCode(0) {}

int GameServer::run() {
    networkHandler->sendGreeting();

    running = true;
    std::thread gameThread(&GameServer::processMoves, this);

    gameThread.join();

    return returnCode;
}

void GameServer::processMoves() {
    while (running) {
        game.displayBoard();
        networkHandler->sendCurrentMove(game.getCurrentPlayer());

        std::string command = networkHandler->receive();
        if (command == "exit") {
            running = false;
            returnCode = 1;
        }

        int x1, y1, x2, y2;
        if (parseMove(command, x1, y1, x2, y2)) {
            std::lock_guard<std::mutex> lock(gameMutex);
            if (game.makeMove(x1, y1, x2, y2)) {
                networkHandler->sendMoveAccepted();
            } else {
                networkHandler->sendInvalidMove();
            }
        } else {
            networkHandler->sendInvalidFormat();
        }

        std::lock_guard<std::mutex> lock(gameMutex);
        if (game.checkWinner()) {
            running = false;
            networkHandler->sendGameOver();
            returnCode = 0;
        }
    }
}

bool GameServer::parseMove(const std::string& input, int& x1, int& y1, int& x2, int& y2) {
    return sscanf(input.c_str(), "%d %d %d %d", &x1, &y1, &x2, &y2) == 4;
}

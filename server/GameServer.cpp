#include "GameServer.h"
#include <cstdio>
#include <thread>

GameServer::GameServer(std::shared_ptr<INetworkHandler> netHandler)
    : networkHandler(std::move(netHandler)), game(), running(false), returnCode(0) {}

int GameServer::run() {
    networkHandler->sendGreeting();
    game.displayBoard();
    networkHandler->sendBoardState(game.getBoard());
    networkHandler->sendCurrentMove(game.getCurrentPlayer());
    running = true;

    networkHandler->startReceiving(this);

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return returnCode;
}

void GameServer::onMessageReceived(const std::string& command) {
    if (command == "exit") {
        running = false;
        returnCode = 1;
        return;
    }

    std::lock_guard<std::mutex> lock(gameMutex);

    int x1, y1, x2, y2;
    if (parseMove(command, x1, y1, x2, y2)) {
        if (game.makeMove(x1, y1, x2, y2)) {
            networkHandler->sendMoveAccepted();
        } else {
            networkHandler->sendInvalidMove();
        }
    } else {
        networkHandler->sendInvalidFormat();
    }

    game.displayBoard();
    networkHandler->sendBoardState(game.getBoard());
    networkHandler->sendCurrentMove(game.getCurrentPlayer());

    if (game.checkWinner()) {
        running = false;
        networkHandler->sendGameOver();
        returnCode = 0;
    }
}

bool GameServer::parseMove(const std::string& input, int& x1, int& y1, int& x2, int& y2) {
    return sscanf(input.c_str(), "%d %d %d %d", &x1, &y1, &x2, &y2) == 4;
}

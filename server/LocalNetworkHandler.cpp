#include "LocalNetworkHandler.h"

#include <thread>

void LocalNetworkHandler::sendGreeting() {
    std::cout << "[Server] Welcome to Checkers! Type moves as: x1 y1 x2 y2" << std::endl;
}

void LocalNetworkHandler::sendCurrentMove(char currentPlayer) {
    std::cout << "[Server] Now there are " << (currentPlayer == 'B' ? "black " : "white ") << "move:" << std::endl;
}

void LocalNetworkHandler::sendMoveAccepted() {
    std::cout << "[Server] Move accepted" << std::endl;
}

void LocalNetworkHandler::sendInvalidMove() {
    std::cout << "[Server] Invalid move" << std::endl;
}

void LocalNetworkHandler::sendInvalidFormat() {
    std::cout << "[Server] Invalid input format" << std::endl;
}

void LocalNetworkHandler::sendGameOver() {
    std::cout << "[Server] Game Over: Winner!" << std::endl;
}

void LocalNetworkHandler::startReceiving(INetworkReceiver* receiver) {
    std::thread([receiver]() {
        while (true) {
            std::string input;
            std::getline(std::cin, input);
            receiver->onMessageReceived(input);
            if (input == "exit") break;
        }
    }).detach();
}
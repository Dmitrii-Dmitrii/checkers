#include "LocalNetworkHandler.h"

void LocalNetworkHandler::send(const std::string& message) {
    std::cout << "[Server] " << message << std::endl;
}

std::string LocalNetworkHandler::receive() {
    std::string input;
    std::getline(std::cin, input);
    return input;
}

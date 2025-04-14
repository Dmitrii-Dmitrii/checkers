#include "GameServer.h"
#include "BoostNetworkHandler.h"
#include <memory>

int main() {
    // Create a Boost network handler with default port 8080
    auto network = std::make_shared<BoostNetworkHandler>();

    // Start the game server with our network handler
    GameServer server(network);
    return server.run();
}
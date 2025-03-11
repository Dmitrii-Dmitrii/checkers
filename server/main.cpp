#include "GameServer.h"
#include "LocalNetworkHandler.h"
#include <memory>

int main() {
    auto network = std::make_shared<LocalNetworkHandler>();
    GameServer server(network);
    return server.run();
}
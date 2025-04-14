#include "GameServer.h"
#include "BoostNetworkHandler.h"
#include <memory>

int main() {
    auto network = std::make_shared<BoostNetworkHandler>();

    GameServer server(network);
    return server.run();
}
#ifndef GAMESERVER_H
#define GAMESERVER_H
#include "INetworkHandler.h"
#include "CheckersGame.h"
#include <memory>

class GameServer {
public:
    explicit GameServer(std::shared_ptr<INetworkHandler> netHandler);
    void run();
private:
    std::shared_ptr<INetworkHandler> networkHandler;
    CheckersGame game;
    bool parseMove(const std::string& input, int& x1, int& y1, int& x2, int& y2);
};

#endif // GAMESERVER_H

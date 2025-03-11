#ifndef GAMESERVER_H
#define GAMESERVER_H

#include "INetworkHandler.h"
#include "CheckersGame.h"
#include <memory>
#include <string>
#include <atomic>
#include <mutex>

class GameServer {
public:
    explicit GameServer(std::shared_ptr<INetworkHandler> netHandler);
    int run();

private:
    void processMoves();
    bool parseMove(const std::string& input, int& x1, int& y1, int& x2, int& y2);
    std::shared_ptr<INetworkHandler> networkHandler;
    CheckersGame game;
    std::atomic<bool> running;
    std::mutex gameMutex;
    int returnCode;
};

#endif // GAMESERVER_H

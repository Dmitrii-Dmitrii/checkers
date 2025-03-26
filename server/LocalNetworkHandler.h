#ifndef LOCALNETWORKHANDLER_H
#define LOCALNETWORKHANDLER_H
#include "INetworkHandler.h"
#include <iostream>

class LocalNetworkHandler : public INetworkHandler {
public:
    void sendGreeting() override;
    void sendCurrentMove(char currentPlayer) override;
    void sendMoveAccepted() override;
    void sendInvalidMove() override;
    void sendInvalidFormat() override;
    void sendGameOver() override;
    void startReceiving(INetworkReceiver* receiver) override;
};

#endif // LOCALNETWORKHANDLER_H
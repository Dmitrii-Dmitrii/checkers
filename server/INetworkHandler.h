#ifndef INETWORKHANDLER_H
#define INETWORKHANDLER_H
#include <string>

class INetworkReceiver {
public:
    virtual void onMessageReceived(const std::string& message) = 0;
    virtual ~INetworkReceiver() = default;
};

class INetworkHandler {
public:
    virtual void sendGreeting() = 0;
    virtual void sendCurrentMove(char currentPlayer) = 0;
    virtual void sendMoveAccepted() = 0;
    virtual void sendInvalidMove() = 0;
    virtual void sendInvalidFormat() = 0;
    virtual void sendGameOver() = 0;
    virtual void startReceiving(INetworkReceiver* receiver) = 0;
    virtual ~INetworkHandler() = default;
};

#endif // INETWORKHANDLER_H

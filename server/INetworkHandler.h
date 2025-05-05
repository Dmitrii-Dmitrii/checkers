#ifndef INETWORKHANDLER_H
#define INETWORKHANDLER_H
#include <string>

class INetworkReceiver {
public:
    virtual void onMessageReceived(const std::string& message) = 0;
    virtual ~INetworkReceiver() = default;
    virtual void sendBoardToClient(uint64_t clientId) = 0;
    virtual void sendCurrentMoveToClient(uint64_t clientId) = 0;
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
    virtual void sendBoardState(const std::vector<std::vector<char>>& board) = 0;
    virtual void sendToPlayer(const std::string& color, const std::string& message) = 0;
    virtual void sendToClient(uint64_t clientId, const std::string& message) = 0;
    virtual ~INetworkHandler() = default;
};

#endif // INETWORKHANDLER_H

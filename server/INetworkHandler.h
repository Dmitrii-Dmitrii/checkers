#ifndef INETWORKHANDLER_H
#define INETWORKHANDLER_H
#include <string>

class INetworkHandler {
public:
    virtual void send(const std::string& message) = 0;
    virtual std::string receive() = 0;
    virtual ~INetworkHandler() = default;
};

#endif // INETWORKHANDLER_H

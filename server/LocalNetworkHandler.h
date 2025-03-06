#ifndef LOCALNETWORKHANDLER_H
#define LOCALNETWORKHANDLER_H
#include "INetworkHandler.h"
#include <iostream>

class LocalNetworkHandler : public INetworkHandler {
public:
    void send(const std::string& message) override;
    std::string receive() override;
};

#endif // LOCALNETWORKHANDLER_H
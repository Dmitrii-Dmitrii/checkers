#include "CheckersClient.h"
#include <iostream>
#include <string>

int main() {
    std::string host = "127.0.0.1";
    int port = 8080;
    
    CheckersClient client(host, port);
    
    if (!client.connect()) {
        return 1;
    }
    
    std::cout << "Connected to checkers server. Type 'exit' to quit." << std::endl;
    std::cout << "Enter moves in format: x1 y1 x2 y2" << std::endl;
    
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") {
            break;
        }
        
        client.sendRawMessage(line);
    }
    
    client.disconnect();
    return 0;
}

#ifndef CHECKERS_CLIENT_H
#define CHECKERS_CLIENT_H

#include <string>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

class CheckersClient {
public:
    CheckersClient(const std::string& host, int port);
    ~CheckersClient();

    bool connect();
    void sendMove(int x1, int y1, int x2, int y2);
    void sendRawMessage(const std::string& message);
    void disconnect();
    const std::vector<std::string>& getReceivedMessages() const;
    bool isConnected() const;

private:
    void readMessages();

    std::string host_;
    int port_;
    std::atomic<bool> running_;
    std::unique_ptr<boost::asio::io_context> io_context_;
    std::unique_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws_stream_;
    std::thread read_thread_;
    std::vector<std::string> receivedMessages_;
};

#endif // CHECKERS_CLIENT_H
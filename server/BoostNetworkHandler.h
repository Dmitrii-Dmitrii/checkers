#ifndef BOOSTNETWORKHANDLER_H
#define BOOSTNETWORKHANDLER_H

#include "INetworkHandler.h"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include <functional>
#include <atomic>
#include <unordered_map>

namespace ip = boost::asio::ip;
namespace ws = boost::beast::websocket;

class BoostNetworkHandler : public INetworkHandler {
public:
    BoostNetworkHandler(int port = 8080);
    ~BoostNetworkHandler() override;

    void sendGreeting() override;
    void sendCurrentMove(char currentPlayer) override;
    void sendMoveAccepted() override;
    void sendInvalidMove() override;
    void sendInvalidFormat() override;
    void sendGameOver() override;
    void startReceiving(INetworkReceiver* receiver) override;
    int getPort() const;
    bool hasActiveConnections();

private:
    using SessionId = uint64_t;

    struct ClientSession {
        std::shared_ptr<ws::stream<boost::beast::tcp_stream>> ws;
        boost::beast::flat_buffer buffer;
        SessionId id;
    };

    void runServer();
    void onAccept(std::shared_ptr<ws::stream<boost::beast::tcp_stream>> ws, 
                 boost::system::error_code ec);
    void doAccept();
    void onHandshake(std::shared_ptr<ClientSession> session, 
                    boost::system::error_code ec);
    void doRead(std::shared_ptr<ClientSession> session);
    void onRead(std::shared_ptr<ClientSession> session, 
               boost::system::error_code ec, 
               std::size_t bytes_transferred);
    void doWrite(std::shared_ptr<ClientSession> session, std::string message);
    void onWrite(std::shared_ptr<ClientSession> session, 
                boost::system::error_code ec, 
                std::size_t bytes_transferred);
    void broadcast(const std::string& message);
    void closeSession(std::shared_ptr<ClientSession> session);

    std::atomic<bool> running;
    boost::asio::io_context io_context;
    ip::tcp::endpoint endpoint;
    std::unique_ptr<ip::tcp::acceptor> acceptor;
    std::thread server_thread;

    std::unordered_map<SessionId, std::shared_ptr<ClientSession>> sessions;
    std::atomic<SessionId> next_session_id{1};

    std::mutex sessions_mutex;
    INetworkReceiver* receiver;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
};

#endif // BOOSTNETWORKHANDLER_H
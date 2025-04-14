#include "BoostNetworkHandler.h"
#include <iostream>
#include <boost/beast/core/buffers_to_string.hpp>
#include <sstream>


BoostNetworkHandler::BoostNetworkHandler(int port)
    : running(false),
      endpoint(ip::tcp::v4(), port),
      receiver(nullptr),
      strand(boost::asio::make_strand(io_context)) {

    acceptor = std::make_unique<ip::tcp::acceptor>(io_context);
}

BoostNetworkHandler::~BoostNetworkHandler() {
    running = false;

    // Close all open sessions
    {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        for (auto& session : sessions) {
            boost::system::error_code ec;
            if (session->ws->is_open()) {
                session->ws->close(ws::close_code::normal, ec);
            }
        }
        sessions.clear();
    }

    if (acceptor && acceptor->is_open()) {
        boost::system::error_code ec;
        acceptor->close(ec);
    }

    // Stop IO context before joining thread
    io_context.stop();

    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void BoostNetworkHandler::startReceiving(INetworkReceiver* receiver_ptr) {
    if (running)
        return;

    this->receiver = receiver_ptr;
    running = true;

    // Initialize the acceptor
    boost::system::error_code ec;
    acceptor->open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Failed to open acceptor: " << ec.message() << std::endl;
        return;
    }

    acceptor->set_option(ip::tcp::acceptor::reuse_address(true), ec);
    if (ec) {
        std::cerr << "Failed to set reuse_address option: " << ec.message() << std::endl;
        return;
    }

    acceptor->bind(endpoint, ec);
    if (ec) {
        std::cerr << "Failed to bind to endpoint: " << ec.message() << std::endl;
        return;
    }

    acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        std::cerr << "Failed to start listening: " << ec.message() << std::endl;
        return;
    }

    doAccept();

    server_thread = std::thread([this]() { runServer(); });

    std::cout << "Server started on port " << endpoint.port() << std::endl;
}

void BoostNetworkHandler::runServer() {
    try {
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
    }
}

void BoostNetworkHandler::doAccept() {
    auto ws = std::make_shared<ws::stream<boost::beast::tcp_stream>>(io_context);

    acceptor->async_accept(
        boost::beast::get_lowest_layer(*ws).socket(),
        boost::asio::bind_executor(strand,
            std::bind(&BoostNetworkHandler::onAccept, this, ws, std::placeholders::_1))
    );
}

void BoostNetworkHandler::onAccept(std::shared_ptr<ws::stream<boost::beast::tcp_stream>> ws,
                                  boost::system::error_code ec) {
    if (ec) {
        std::cerr << "Accept failed: " << ec.message() << std::endl;
    } else {
        // Create a new session for this connection
        auto session = std::make_shared<ClientSession>();
        session->ws = ws;

        // Set websocket options
        session->ws->set_option(ws::stream_base::timeout::suggested(boost::beast::role_type::server));
        session->ws->set_option(ws::stream_base::decorator(
            [](ws::response_type& res) {
                res.set(boost::beast::http::field::server, "CheckersServer");
            }
        ));

        // Accept the websocket handshake
        session->ws->async_accept(
            std::bind(
                &BoostNetworkHandler::onHandshake,
                this,
                session,
                std::placeholders::_1
            )
        );
    }

    // Accept the next connection
    if (running) {
        doAccept();
    }
}

void BoostNetworkHandler::onHandshake(std::shared_ptr<ClientSession> session,
                                     boost::system::error_code ec) {
    if (ec) {
        std::cerr << "Handshake failed: " << ec.message() << std::endl;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        sessions.push_back(session);
    }

    std::cout << "New client connected" << std::endl;

    // Send greeting to the newly connected client
    sendGreeting();

    // Start reading messages from this client
    doRead(session);
}

void BoostNetworkHandler::doRead(std::shared_ptr<ClientSession> session) {
    session->ws->async_read(
        session->buffer,
        std::bind(
            &BoostNetworkHandler::onRead,
            this,
            session,
            std::placeholders::_1,
            std::placeholders::_2
        )
    );
}

void BoostNetworkHandler::onRead(std::shared_ptr<ClientSession> session,
                                boost::system::error_code ec,
                                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec == ws::error::closed) {
        std::cout << "Connection closed by client" << std::endl;
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = std::find(sessions.begin(), sessions.end(), session);
        if (it != sessions.end()) {
            sessions.erase(it);
        }
        return;
    }

    if (ec == ws::error::closed || ec) {
        if (ec == ws::error::closed) {
            std::cout << "Connection closed by client" << std::endl;
        } else {
            std::cerr << "Read failed: " << ec.message() << std::endl;
        }

        closeSession(session);
        return;
    }

    // Convert message to string
    std::string message = boost::beast::buffers_to_string(session->buffer.data());
    session->buffer.consume(session->buffer.size());

    // Forward the message to the game server
    if (receiver) {
        receiver->onMessageReceived(message);
    }

    // Continue reading from this client
    if (session->ws->is_open()) {
        doRead(session);
    }
}

void BoostNetworkHandler::doWrite(std::shared_ptr<ClientSession> session, std::string message) {
    session->ws->async_write(
        boost::asio::buffer(message),
        std::bind(
            &BoostNetworkHandler::onWrite,
            this,
            session,
            std::placeholders::_1,
            std::placeholders::_2
        )
    );
}

void BoostNetworkHandler::onWrite(std::shared_ptr<ClientSession> session,
                                 boost::system::error_code ec,
                                 std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        std::cerr << "Write failed: " << ec.message() << std::endl;
        boost::system::error_code close_ec;
        session->ws->close(ws::close_code::normal, close_ec);
    }
}

void BoostNetworkHandler::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(sessions_mutex);
    for (auto& session : sessions) {
        boost::asio::post(
            strand,  // Use strand instead of direct io_context
            [this, session, message]() {
                this->doWrite(session, message);
            }
        );
    }
}

// Helper function to create a simple JSON-like message
std::string createJsonMessage(const std::string& type, const std::string& message,
                              const std::string& extraKey = "", const std::string& extraValue = "") {
    std::ostringstream oss;
    oss << "{\"type\":\"" << type << "\",\"message\":\"" << message << "\"";

    if (!extraKey.empty() && !extraValue.empty()) {
        oss << ",\"" << extraKey << "\":\"" << extraValue << "\"";
    }

    oss << "}";
    return oss.str();
}

// INetworkHandler implementation
void BoostNetworkHandler::sendGreeting() {
    broadcast(createJsonMessage("greeting", "Welcome to Checkers! Type moves as: x1 y1 x2 y2"));
}

void BoostNetworkHandler::sendCurrentMove(char currentPlayer) {
    std::string message = std::string("Now there are ") +
                         (currentPlayer == 'B' ? "black " : "white ") +
                         "move:";

    broadcast(createJsonMessage("current_move", message, "player", std::string(1, currentPlayer)));
}

void BoostNetworkHandler::sendMoveAccepted() {
    broadcast(createJsonMessage("move_accepted", "Move accepted"));
}

void BoostNetworkHandler::sendInvalidMove() {
    broadcast(createJsonMessage("invalid_move", "Invalid move"));
}

void BoostNetworkHandler::sendInvalidFormat() {
    broadcast(createJsonMessage("invalid_format", "Invalid input format"));
}

void BoostNetworkHandler::sendGameOver() {
    broadcast(createJsonMessage("game_over", "Game Over: Winner!"));
}

void BoostNetworkHandler::closeSession(std::shared_ptr<ClientSession> session) {
    boost::asio::post(strand, [this, session]() {
        boost::system::error_code ec;
        if (session->ws->is_open()) {
            session->ws->close(ws::close_code::normal, ec);
        }

        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = std::find(sessions.begin(), sessions.end(), session);
        if (it != sessions.end()) {
            sessions.erase(it);
        }
    });
}
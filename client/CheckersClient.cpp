#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <regex>

#include "SimpleJsonParser.h"
#include "CheckersClient.h"

namespace ip = boost::asio::ip;
namespace ws = boost::beast::websocket;

CheckersClient::CheckersClient(const std::string &host, int port)
    : host_(host), port_(port), running_(false) {
}

CheckersClient::~CheckersClient() {
    disconnect();
}

bool CheckersClient::connect() {
    try {
        io_context_ = std::make_unique<boost::asio::io_context>();

        ip::tcp::resolver resolver(*io_context_);
        auto const results = resolver.resolve(host_, std::to_string(port_));

        ws_stream_ = std::make_unique<ws::stream<boost::beast::tcp_stream> >(*io_context_);
        boost::beast::get_lowest_layer(*ws_stream_).connect(results);

        ws_stream_->handshake(host_ + ":" + std::to_string(port_), "/");

        std::cout << "Connected to server at " << host_ << ":" << port_ << std::endl;
        running_ = true;

        read_thread_ = std::thread([this]() { readMessages(); });

        return true;
    } catch (const std::exception &e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

void CheckersClient::sendMove(int x1, int y1, int x2, int y2) {
    if (!running_) return;

    try {
        std::string move = std::to_string(x1) + " " +
                           std::to_string(y1) + " " +
                           std::to_string(x2) + " " +
                           std::to_string(y2);

        ws_stream_->write(boost::asio::buffer(move));
    } catch (const std::exception &e) {
        std::cerr << "Failed to send move: " << e.what() << std::endl;
    }
}

void CheckersClient::sendRawMessage(const std::string &message) {
    if (!running_) return;

    try {
        ws_stream_->write(boost::asio::buffer(message));
    } catch (const std::exception &e) {
        std::cerr << "Failed to send message: " << e.what() << std::endl;
    }
}

void CheckersClient::disconnect() {
    if (!running_) return;

    running_ = false;

    try {
        ws_stream_->close(ws::close_code::normal);
    } catch (const std::exception &e) {
        std::cerr << "Error during disconnect: " << e.what() << std::endl;
    }

    if (read_thread_.joinable()) {
        read_thread_.join();
    }

    io_context_->stop();
}

void CheckersClient::readMessages() {
    try {
        boost::beast::flat_buffer buffer;

        while (running_) {
            boost::system::error_code ec;
            ws_stream_->read(buffer, ec);

            if (ec) {
                if (ec == boost::beast::websocket::error::closed) {
                    std::cout << "Connection closed by server" << std::endl;
                } else if (ec == boost::asio::error::operation_aborted) {
                    std::cout << "Operation aborted" << std::endl;
                } else {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
                running_ = false;
                break;
            }

            std::string json_str = boost::beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());

            try {
                std::map<std::string, std::string> parsed;
                if (SimpleJsonParser::parse(json_str, parsed)) {
                    std::string type = parsed["type"];
                    std::string message = parsed["message"];

                    if (type == "greeting") {
                        std::cout << "Server: " << message << std::endl;
                    } else if (type == "current_move") {
                        std::string player = parsed["player"];
                        std::cout << "Server: " << message << std::endl;
                    } else if (type == "move_accepted") {
                        std::cout << "Server: " << message << std::endl;
                    } else if (type == "invalid_move") {
                        std::cout << "Server: " << message << std::endl;
                    } else if (type == "invalid_format") {
                        std::cout << "Server: " << message << std::endl;
                    } else if (type == "game_over") {
                        std::cout << "Server: " << message << std::endl;
                        std::cout << "Game over! Press Enter to exit." << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    } else {
                        std::cout << "Unknown message type: " << type << ": " << message << std::endl;
                    }
                } else {
                    std::cout << "Raw message: " << json_str << std::endl;
                }
            } catch (const std::exception &e) {
                std::cerr << "Failed to parse server message: " << e.what() << std::endl;
                std::cout << "Raw message: " << json_str << std::endl;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Unexpected error in read thread: " << e.what() << std::endl;
        running_ = false;
    }
}

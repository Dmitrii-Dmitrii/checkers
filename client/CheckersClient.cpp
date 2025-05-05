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

        std::cout << "Sending move: " << move << std::endl;

        boost::beast::error_code ec;
        ws_stream_->write(boost::asio::buffer(move), ec);

        if (ec) {
            std::cerr << "Error sending move: " << ec.message() << std::endl;
        }
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

                if (!running_) break;

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            std::string json_str = boost::beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());

            std::cout << "Received: " << json_str << std::endl;

            std::map<std::string, std::string> parsed;
            if (SimpleJsonParser::parse(json_str, parsed)) {
                auto type = parsed["type"];
            }

            receivedMessages_.push_back(json_str);

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception &e) {
        std::cerr << "Unexpected error in read thread: " << e.what() << std::endl;
        running_ = false;
    }
}

const std::vector<std::string>&  CheckersClient::getReceivedMessages() const {
    return receivedMessages_;
}

bool  CheckersClient::isConnected() const {
    return running_;
}
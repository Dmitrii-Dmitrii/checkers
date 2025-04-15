#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../client/CheckersClient.h"
#include "../server/BoostNetworkHandler.h"
#include "../client/SimpleJsonParser.h"
#include <thread>
#include <chrono>

using ::testing::_;
using ::testing::AtLeast;

class MockNetworkReceiver : public INetworkReceiver {
public:
    MOCK_METHOD(void, onMessageReceived, (const std::string& message), (override));
};

TEST(NetworkTest, ConnectAndDisconnect) {
    BoostNetworkHandler server(0);
    server.startReceiving(nullptr);
    int port = server.getPort();

    CheckersClient client("localhost", port);
    bool connected = client.connect();
    ASSERT_TRUE(connected);

    client.disconnect();
    ASSERT_FALSE(client.isConnected());
}

TEST(NetworkTest, SendMoveCorrectFormat) {
    BoostNetworkHandler server(0);
    MockNetworkReceiver receiver;
    server.startReceiving(&receiver);

    CheckersClient client("localhost", server.getPort());
    ASSERT_TRUE(client.connect());

    EXPECT_CALL(receiver, onMessageReceived("1 2 3 4")).Times(1);

    client.sendMove(1, 2, 3, 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST(NetworkTest, ReceiveMessageFromServer) {
    BoostNetworkHandler server(0);
    server.startReceiving(nullptr);

    CheckersClient client("localhost", server.getPort());
    ASSERT_TRUE(client.connect());

    // Сервер отправляет приветствие при подключении
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto messages = client.getReceivedMessages();
    ASSERT_FALSE(messages.empty());

    bool greetingFound = false;
    for (const auto& msg : messages) {
        std::map<std::string, std::string> parsed;
        if (SimpleJsonParser::parse(msg, parsed) && parsed["type"] == "greeting") {
            greetingFound = true;
            break;
        }
    }
    ASSERT_TRUE(greetingFound);
}

TEST(BoostNetworkHandlerTest, StartServerAndAcceptConnection) {
    BoostNetworkHandler server(0);
    server.startReceiving(nullptr);
    ASSERT_NE(server.getPort(), -1);

    CheckersClient client("localhost", server.getPort());
    bool connected = client.connect();
    ASSERT_TRUE(connected);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(server.hasActiveConnections());
}

TEST(BoostNetworkHandlerTest, BroadcastMessageToClients) {
    BoostNetworkHandler server(0);
    server.startReceiving(nullptr);

    CheckersClient client1("localhost", server.getPort());
    CheckersClient client2("localhost", server.getPort());
    ASSERT_TRUE(client1.connect());
    ASSERT_TRUE(client2.connect());

    server.sendGreeting();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_GE(client1.getReceivedMessages().size(), 1);
    ASSERT_GE(client2.getReceivedMessages().size(), 1);
}

TEST(BoostNetworkHandlerTest, HandleInvalidMessageFormat) {
    BoostNetworkHandler server(0);
    server.startReceiving(nullptr);

    CheckersClient client("localhost", server.getPort());
    ASSERT_TRUE(client.connect());

    client.sendRawMessage("invalid_message");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto messages = client.getReceivedMessages();
    bool invalidFormatFound = false;
    for (const auto& msg : messages) {
        std::map<std::string, std::string> parsed;
        if (SimpleJsonParser::parse(msg, parsed) && parsed["type"] == "invalid_format") {
            invalidFormatFound = true;
            break;
        }
    }
    ASSERT_TRUE(invalidFormatFound);
}

TEST(BoostNetworkHandlerTest, SendGameOverAndCloseConnections) {
    BoostNetworkHandler server(0);
    server.startReceiving(nullptr);

    CheckersClient client("localhost", server.getPort());
    ASSERT_TRUE(client.connect());

    server.sendGameOver();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto messages = client.getReceivedMessages();
    bool gameOverFound = false;
    for (const auto& msg : messages) {
        std::map<std::string, std::string> parsed;
        if (SimpleJsonParser::parse(msg, parsed) && parsed["type"] == "game_over") {
            gameOverFound = true;
            break;
        }
    }
    ASSERT_TRUE(gameOverFound);
    ASSERT_FALSE(client.isConnected());
}
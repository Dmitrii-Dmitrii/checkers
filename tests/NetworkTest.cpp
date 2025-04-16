#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>

#include "../client/CheckersClient.h"
#include "../server/BoostNetworkHandler.h"
#include "../client/SimpleJsonParser.h"


class MockNetworkReceiver : public INetworkReceiver {
public:
    MOCK_METHOD1(onMessageReceived, void(const std::string& message));
};

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

class CheckersClientTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

class BoostNetworkHandlerTest : public ::testing::Test {
protected:
    std::unique_ptr<MockNetworkReceiver> mockReceiver;
    std::unique_ptr<BoostNetworkHandler> networkHandler;

    void SetUp() override {
        mockReceiver = std::make_unique<MockNetworkReceiver>();
    }

    void TearDown() override {
        if (networkHandler) {
            networkHandler.reset();
        }
    }
};

TEST_F(CheckersClientTest, ConnectToServer) {
    BoostNetworkHandler server(0);
    MockNetworkReceiver mockReceiver;
    server.startReceiving(&mockReceiver);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int port = server.getPort();
    ASSERT_GT(port, 0);

    CheckersClient client("localhost", port);
    bool connected = client.connect();

    EXPECT_TRUE(connected);
    EXPECT_TRUE(client.isConnected());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto& messages = client.getReceivedMessages();
    ASSERT_FALSE(messages.empty());

    std::map<std::string, std::string> parsed;
    ASSERT_TRUE(SimpleJsonParser::parse(messages[0], parsed));
    EXPECT_EQ(parsed["type"], "greeting");

    client.disconnect();
}

TEST_F(CheckersClientTest, SendMove) {
    BoostNetworkHandler server(0);
    MockNetworkReceiver mockReceiver;

    EXPECT_CALL(mockReceiver, onMessageReceived("1 2 3 4"))
        .Times(1);

    server.startReceiving(&mockReceiver);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int port = server.getPort();

    CheckersClient client("localhost", port);
    client.connect();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.sendMove(1, 2, 3, 4);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.disconnect();
}

TEST_F(CheckersClientTest, SendRawMessage) {
    BoostNetworkHandler server(0);
    MockNetworkReceiver mockReceiver;

    EXPECT_CALL(mockReceiver, onMessageReceived("custom raw message"))
        .Times(1);

    server.startReceiving(&mockReceiver);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int port = server.getPort();

    CheckersClient client("localhost", port);
    client.connect();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.sendRawMessage("custom raw message");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.disconnect();
}

TEST_F(CheckersClientTest, DisconnectFromServer) {
    BoostNetworkHandler server(0);
    MockNetworkReceiver mockReceiver;
    server.startReceiving(&mockReceiver);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int port = server.getPort();

    CheckersClient client("localhost", port);
    client.connect();

    EXPECT_TRUE(client.isConnected());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.disconnect();

    EXPECT_FALSE(client.isConnected());
}

TEST_F(BoostNetworkHandlerTest, StartServer) {
    networkHandler = std::make_unique<BoostNetworkHandler>(0);
    networkHandler->startReceiving(mockReceiver.get());

    int port = networkHandler->getPort();
    EXPECT_GT(port, 0);

    EXPECT_FALSE(networkHandler->hasActiveConnections());
}

TEST_F(BoostNetworkHandlerTest, ClientConnection) {
    networkHandler = std::make_unique<BoostNetworkHandler>(0);
    networkHandler->startReceiving(mockReceiver.get());

    int port = networkHandler->getPort();

    CheckersClient client("localhost", port);
    bool connected = client.connect();
    EXPECT_TRUE(connected);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_TRUE(networkHandler->hasActiveConnections());

    client.disconnect();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(networkHandler->hasActiveConnections());
}

TEST_F(BoostNetworkHandlerTest, BroadcastMessages) {
    networkHandler = std::make_unique<BoostNetworkHandler>(0);
    networkHandler->startReceiving(mockReceiver.get());

    int port = networkHandler->getPort();

    CheckersClient client("localhost", port);
    client.connect();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    const auto& initialMessages = client.getReceivedMessages();
    size_t initialCount = initialMessages.size();

    networkHandler->sendCurrentMove('B');
    networkHandler->sendMoveAccepted();
    networkHandler->sendInvalidMove();
    networkHandler->sendInvalidFormat();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    const auto& messages = client.getReceivedMessages();
    EXPECT_GT(messages.size(), initialCount);

    bool foundCurrentMove = false;
    bool foundMoveAccepted = false;

    for (size_t i = initialCount; i < messages.size(); i++) {
        std::map<std::string, std::string> parsed;
        if (SimpleJsonParser::parse(messages[i], parsed)) {
            if (parsed["type"] == "current_move" && parsed["player"] == "B") {
                foundCurrentMove = true;
            } else if (parsed["type"] == "move_accepted") {
                foundMoveAccepted = true;
            }
        }
    }

    EXPECT_TRUE(foundCurrentMove);
    EXPECT_TRUE(foundMoveAccepted);

    client.disconnect();
}

TEST_F(BoostNetworkHandlerTest, GameOverClosesSessions) {
    networkHandler = std::make_unique<BoostNetworkHandler>(0);
    networkHandler->startReceiving(mockReceiver.get());

    int port = networkHandler->getPort();

    CheckersClient client("localhost", port);
    client.connect();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_TRUE(networkHandler->hasActiveConnections());

    networkHandler->sendGameOver();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_FALSE(networkHandler->hasActiveConnections());
    EXPECT_FALSE(client.isConnected());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
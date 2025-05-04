cmake_minimum_required(VERSION 3.30)
project(checkers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_OSX_ARCHITECTURES "arm64")

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Core LinguistTools)

add_executable(checkers
        server/main.cpp
        server/INetworkHandler.h
        server/LocalNetworkHandler.h
        server/LocalNetworkHandler.cpp
        server/CheckersGame.h
        server/CheckersGame.cpp
        server/GameServer.h
        server/GameServer.cpp
        server/BoostNetworkHandler.cpp
        server/BoostNetworkHandler.h
)
target_include_directories(checkers PRIVATE
        ${PROJECT_SOURCE_DIR}/server
)
target_link_libraries(checkers PRIVATE
        ${Boost_LIBRARIES}
        pthread
)

add_library(CheckersCore STATIC
        client/CheckersClient.cpp
        client/SimpleJsonParser.cpp
)
target_include_directories(CheckersCore PUBLIC
        ${PROJECT_SOURCE_DIR}/client
)
target_link_libraries(CheckersCore PUBLIC
        ${Boost_LIBRARIES}
        pthread
)

add_executable(CheckersClient
        client/main.cpp
        client/CheckersClient.h
        client/CheckersClient.cpp
        client/SimpleJsonParser.h
        client/SimpleJsonParser.cpp
)
target_include_directories(CheckersClient PRIVATE
        ${PROJECT_SOURCE_DIR}/client
)
target_link_libraries(CheckersClient PRIVATE
        ${Boost_LIBRARIES}
        pthread
)

set(GUI_SOURCES
        qt/main.cpp
        qt/AppEventFilter.h
        qt/AppEventFilter.cpp
        qt/MainWindow.h
        qt/MainWindow.cpp
        qt/MyButton.h
        qt/MyButton.cpp
        qt/resource.qrc
        qt/BoardWidget.cpp
        qt/BoardWidget.h
)
add_executable(CheckersQt ${GUI_SOURCES})

target_include_directories(CheckersQt PRIVATE
        ${PROJECT_SOURCE_DIR}/qt
)

target_link_libraries(CheckersQt PRIVATE
        CheckersCore
        Qt6::Widgets
        Qt6::Network
        Qt6::Core
)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
enable_testing()

add_executable(checkers_game_tests
        tests/CheckersGameTest.cpp
        server/CheckersGame.h
        server/CheckersGame.cpp
)

target_link_libraries(checkers_game_tests
        GTest::Main
        ${Boost_LIBRARIES}
        pthread
)

add_test(NAME GameTests COMMAND checkers_game_tests)

add_executable(parser_tests
        tests/SimpleJsonParserTest.cpp
        client/SimpleJsonParser.h
        client/SimpleJsonParser.cpp
)

target_link_libraries(parser_tests
        GTest::Main
        pthread
)

add_test(NAME parser_tests COMMAND parser_tests)

add_executable(network_tests
        tests/NetworkTest.cpp
        client/CheckersClient.h
        client/CheckersClient.cpp
        client/SimpleJsonParser.h
        client/SimpleJsonParser.cpp
        server/BoostNetworkHandler.h
        server/BoostNetworkHandler.cpp
)

target_link_libraries(network_tests
        GTest::Main
        GTest::gmock
        GTest::gmock_main
        ${Boost_LIBRARIES}
        pthread
)

add_test(NAME network_tests COMMAND network_tests)
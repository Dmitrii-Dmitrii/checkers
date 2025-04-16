cmake_minimum_required(VERSION 3.30)
project(checkers)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

add_executable(checkers server/main.cpp
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

target_link_libraries(checkers
        ${Boost_LIBRARIES}
        pthread
)

add_executable(CheckersClient client/main.cpp
        client/CheckersClient.h
        client/CheckersClient.cpp
        client/SimpleJsonParser.h
        client/SimpleJsonParser.cpp
)

target_link_libraries(CheckersClient
        ${Boost_LIBRARIES}
        pthread
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
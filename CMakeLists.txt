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

add_executable(CheckersClient
        client/CheckersClient.cpp
)

target_link_libraries(CheckersClient
        ${Boost_LIBRARIES}
        pthread
)

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(checkers_tests
        tests/CheckersGameTest.cpp
        server/CheckersGame.h
        server/CheckersGame.cpp
)

target_link_libraries(checkers_tests
        ${GTEST_LIBRARIES}
        GTest::Main
        pthread
)

enable_testing()
add_test(NAME CheckersTests COMMAND checkers_tests)
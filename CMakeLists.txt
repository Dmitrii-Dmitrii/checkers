cmake_minimum_required(VERSION 3.30)
project(checkers)

set(CMAKE_CXX_STANDARD 20)

add_executable(checkers server/main.cpp
        server/INetworkHandler.h
        server/LocalNetworkHandler.h
        server/LocalNetworkHandler.cpp
        server/CheckersGame.h
        server/CheckersGame.cpp
        server/GameServer.h
        server/GameServer.cpp
)

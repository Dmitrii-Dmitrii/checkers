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

find_package(GTest REQUIRED)

# Подключение заголовочных файлов Google Test
include_directories(${GTEST_INCLUDE_DIRS})

# Отдельный исполняемый файл для тестов
add_executable(checkers_tests
        tests/CheckersGameTest.cpp
        server/CheckersGame.h
        server/CheckersGame.cpp
)

# Линкуем тесты с Google Test и потоками
target_link_libraries(checkers_tests
        ${GTEST_LIBRARIES}
        GTest::Main
        pthread
)

# Добавляем тесты в CTest
enable_testing()
add_test(NAME CheckersTests COMMAND checkers_tests)
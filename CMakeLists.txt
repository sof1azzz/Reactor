cmake_minimum_required(VERSION 3.16)

project(Reactor)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcpepoll
    tcpepoll.cpp
    Socket.cpp
    InetAddress.cpp
    Epoll.cpp
    Channel.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    TcpServer.cpp
    TcpConnection.cpp
    Buffer.cpp
)

target_link_libraries(tcpepoll pthread)

add_executable(client
    client.cpp
    Socket.cpp
    InetAddress.cpp
    Epoll.cpp
    Channel.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    TcpServer.cpp
    TcpConnection.cpp
    Buffer.cpp
)

target_link_libraries(client pthread)

add_custom_target(tests
  COMMAND ${CMAKE_SOURCE_DIR}/tests/test_client.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Running Python test"
)
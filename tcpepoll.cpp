#include "Buffer.h"
#include "Callbacks.h"
#include "TcpServer.h"
#include <chrono>
#include <iostream>
#include <thread>

// 1. 连接建立和断开的回调
void onConnection(const TcpConnectionPtr &conn) {
  if (conn->connected()) {
    std::cout << "onConnection(): new connection [" << conn->name() << "] from "
              << conn->peerAddress().getIp() << ":"
              << conn->peerAddress().getPort() << std::endl;
  } else {
    std::cout << "onConnection(): connection [" << conn->name() << "] is down."
              << std::endl;
  }
}

// 2. 可读事件回调
void onMessage(const TcpConnectionPtr &conn, Buffer &buf) {
  std::string msg(buf.retrieveAllAsString());
  std::cout << "onMessage(): received " << msg.size()
            << " bytes from connection [" << conn->name() << "]: " << msg
            << std::endl;

  // 正常情况：在I/O线程中直接发送
  // conn->send(msg); // 回显

  // 测试跨线程调用：从另一个线程发送
  std::thread([conn, msg]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    conn->send(msg);
  }).detach();
}

// 3. 数据发送完毕的回调
void onWriteComplete(const TcpConnectionPtr &conn) {
  std::cout << "onWriteComplete(): write complete for connection ["
            << conn->name() << "]" << std::endl;
}

// 4. 连接关闭的回调 (通常由框架内部使用)
void onClose(const TcpConnectionPtr &conn) {
  std::cout << "onClose(): connection [" << conn->name() << "] is closing."
            << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    logError("Usage: " + std::string(argv[0]) + " <ip> <port>", "main");
    return 1;
  }
  const char *ip = argv[1];
  int port = atoi(argv[2]);

  TcpServer tcpServer(ip, port);

  // 设置线程数
  tcpServer.setThreadNum(4);

  // 设置全部四个回调函数
  tcpServer.setConnectionCallback(onConnection);
  tcpServer.setMessageCallback(onMessage);
  tcpServer.setWriteCompleteCallback(onWriteComplete);
  tcpServer.setCloseCallback(onClose);

  tcpServer.start();
  return 0;
}

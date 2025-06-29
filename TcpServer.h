#pragma once

#include "Callbacks.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Socket.h"
#include "TcpConnection.h"
#include <map>
#include <memory>
#include <string>

class TcpConnection;

class TcpServer {
public:
  TcpServer(const std::string &ip, const uint16_t port);
  ~TcpServer();

  void start();
  void stop();

  void setThreadNum(int numThreads);

  // 获取连接
  std::shared_ptr<TcpConnection> getConnection(const std::string &name) {
    return connections_[name];
  }

  // 设置回调函数
  void setConnectionCallback(const TcpConnectionCallback &cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }
  void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

  // 处理新连接
private:
  void handleNewConnection();
  void handleWrite();
  void removeConnection(const std::shared_ptr<TcpConnection> &conn);

private:
  std::unique_ptr<EventLoop> eventLoop_;
  std::unique_ptr<EventLoopThreadPool> threadPool_;
  const std::string ip_;
  const uint16_t port_;
  std::unique_ptr<Socket> listensock_;
  std::unique_ptr<Channel> listen_channel_;
  TcpConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  CloseCallback closeCallback_;
  std::map<std::string, std::shared_ptr<TcpConnection>> connections_;
  InetAddress server_addr_;
  std::mutex connections_mutex_;
};
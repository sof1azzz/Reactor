#include "TcpServer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "TcpConnection.h"
#include <cstring>
#include <memory>

TcpServer::TcpServer(const std::string &ip, const uint16_t port)
    : ip_(ip), port_(port), listensock_(new Socket(createNonblockingSocket())),
      connectionCallback_(nullptr), messageCallback_(nullptr),
      writeCompleteCallback_(nullptr), closeCallback_(nullptr), connections_(),
      server_addr_(ip, port) {
  listensock_->setReuseAddr(true);
  listensock_->setReusePort(true);
  listensock_->setTcpNoDelay(true);
  listensock_->setKeepAlive(true);
  listensock_->bind(InetAddress(ip_, port_));
  listensock_->listen();

  // 创建EventLoop
  eventLoop_ = new EventLoop();
  // 为监听socket创建Channel
  listen_channel_ = new Channel(listensock_->getFd(), eventLoop_->getEpoll());
  // 设置监听socket的回调函数
  listen_channel_->setReadCallback([this]() { handleNewConnection(); });
  // 启用读事件
  listen_channel_->enableReading();
}

TcpServer::~TcpServer() {
  listensock_->close();
  delete listensock_;
  delete listen_channel_;
  delete eventLoop_;
}

void TcpServer::stop() {
  listensock_->close();
  delete listensock_;
  delete listen_channel_;
  delete eventLoop_;
}

void TcpServer::start() { eventLoop_->run(); }

// 处理新连接的回调函数。并设置客户端数据处理回调
void TcpServer::handleNewConnection() {
  log("Handling new connection...", "handleNewConnection");
  InetAddress client_addr;
  int clientFd = listensock_->accept(client_addr);
  if (clientFd == -1) {
    logError("Error accepting new connection", "handleNewConnection");
    return;
  }

  // 创建TcpConnection
  std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(
      eventLoop_,
      client_addr.getIp() + ":" + std::to_string(client_addr.getPort()),
      clientFd, server_addr_, client_addr);

  // 设置回调函数
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(closeCallback_);
  // 连接建立
  conn->connectEstablished();
  // 存到map中
  connections_[client_addr.getIp() + ":" +
               std::to_string(client_addr.getPort())] = conn;

  log("Client connected from " + client_addr.getIp() + ":" +
          std::to_string(client_addr.getPort()),
      "handleNewConnection");
}
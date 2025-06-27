#include "TcpServer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "TcpConnection.h"
#include <cstring>
#include <functional>
#include <memory>

TcpServer::TcpServer(const std::string &ip, const uint16_t port)
    : ip_(ip), port_(port),
      listensock_(std::make_unique<Socket>(createNonblockingSocket())),
      eventLoop_(std::make_unique<EventLoop>()), server_addr_(ip, port),
      connectionCallback_(nullptr), messageCallback_(nullptr),
      writeCompleteCallback_(nullptr), connections_() {
  listensock_->setReuseAddr(true);
  listensock_->setReusePort(true);
  listensock_->setTcpNoDelay(true);
  listensock_->setKeepAlive(true);
  listensock_->bind(server_addr_);
  listensock_->listen();

  // 为监听socket创建Channel
  listen_channel_ =
      std::make_unique<Channel>(listensock_->getFd(), eventLoop_->getEpoll());
  // 设置监听socket的回调函数
  listen_channel_->setReadCallback([this]() { handleNewConnection(); });

  listen_channel_->setWriteCallback([this]() { handleWrite(); });
  // 启用读事件
  listen_channel_->enableReading();
}

TcpServer::~TcpServer() {
  // 智能指针会自动释放资源，无需手动delete
}

void TcpServer::stop() {
  // 智能指针会自动释放资源，无需手动delete
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
      eventLoop_.get(),
      client_addr.getIp() + ":" + std::to_string(client_addr.getPort()),
      clientFd, server_addr_, client_addr);

  // 设置回调函数
  // 设置TcpConnection的连接回调为TcpServer::connectionCallback_,来自于main
  conn->setConnectionCallback(connectionCallback_);

  // 待确定
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);

  // 设置TcpConnection的关闭回调为TcpServer::removeConnection
  conn->setCloseCallback(
      [this](const TcpConnectionPtr &conn) { removeConnection(conn); });
  // 连接建立
  conn->connectEstablished();
  // 存到map中
  connections_[client_addr.getIp() + ":" +
               std::to_string(client_addr.getPort())] = conn;

  log("Client connected from " + client_addr.getIp() + ":" +
          std::to_string(client_addr.getPort()),
      "handleNewConnection");
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
  log("Removing connection: " + conn->name(), "removeConnection");

  // 从map中移除连接
  connections_.erase(conn->name());
  conn->connectDestroyed();

  if (connectionCallback_) {
    connectionCallback_(conn);
  }
}

void TcpServer::handleWrite() {
  log("Handling write event...", "handleWrite");
}
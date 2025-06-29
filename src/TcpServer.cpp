#include "../include/TcpServer.h"
#include "../include/Channel.h"
#include "../include/EventLoop.h"
#include "../include/EventLoopThreadPool.h"
#include "../include/InetAddress.h"
#include "../include/Socket.h"
#include "../include/TcpConnection.h"
#include <cstring>
#include <functional>
#include <memory>
#include <type_traits>

TcpServer::TcpServer(const std::string &ip, const uint16_t port)
    : eventLoop_(std::make_unique<EventLoop>()),
      threadPool_(std::make_unique<EventLoopThreadPool>(eventLoop_.get(),
                                                        4 /*numThreads*/)),
      ip_(ip), port_(port), listensock_(std::make_unique<Socket>()),
      listen_channel_(std::make_unique<Channel>(listensock_->getFd(),
                                                eventLoop_->getPoller())),
      server_addr_(ip, port), connectionCallback_(nullptr),
      messageCallback_(nullptr), writeCompleteCallback_(nullptr),
      connections_() {
  listensock_->bind(server_addr_);
  listensock_->listen();
  listensock_->setReuseAddr(true);
  listensock_->setReusePort(true);
  listensock_->setTcpNoDelay(true);
  listensock_->setKeepAlive(true);

  // 为监听socket创建Channel
  listen_channel_->setWriteCallback([this]() { handleWrite(); });
  // 设置监听socket的读回调
  listen_channel_->setReadCallback([this]() { handleNewConnection(); });
  listen_channel_->enableReading();
}

TcpServer::~TcpServer() {
  // 智能指针会自动释放资源，无需手动delete
}

void TcpServer::stop() {
  // 智能指针会自动释放资源，无需手动delete
}

void TcpServer::setThreadNum(int numThreads) {
  threadPool_ =
      std::make_unique<EventLoopThreadPool>(eventLoop_.get(), numThreads);
}

void TcpServer::start() {
  // 启动线程池
  threadPool_->start();
  // 启动事件循环
  eventLoop_->loop();
}

// 判断T是否是TcpConnection的引用
template <typename T>
concept IsTcpConnRef =
    std::is_same_v<std::remove_cvref_t<T>, std::shared_ptr<TcpConnection>>;

// 处理新连接的回调函数。并设置客户端数据处理回调
void TcpServer::handleNewConnection() {
  // 获取客户端地址
  InetAddress peerAddr;
  int connfd = listensock_->accept(peerAddr);
  if (connfd < 0) {
    logError(strerror(errno), "handleNewConnection");
    return;
  }

  EventLoop *ioLoop = threadPool_->getNextLoop();

  // 创建TcpConnection
  std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(
      ioLoop, "conn" + std::to_string(connfd), connfd,
      InetAddress(Socket::getLocalAddr(connfd)), peerAddr);

  // 设置回调函数
  // 设置TcpConnection的连接回调为TcpServer::connectionCallback_,来自于main
  conn->setConnectionCallback(connectionCallback_);

  // 待确定
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);

  // 设置TcpConnection的关闭回调为TcpServer::removeConnection
  conn->setCloseCallback([this]<IsTcpConnRef T>(T &&PH1) {
    removeConnection(std::forward<T>(PH1));
  });

  // 在I/O线程中调用connectEstablished
  ioLoop->queueInLoop([conn]() { conn->connectEstablished(); });

  // 存到map中
  {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_[conn->name()] = conn;
  }
}

void TcpServer::removeConnection(const std::shared_ptr<TcpConnection> &conn) {
  log("Removing connection: " + conn->name(), "removeConnection");

  // 在I/O线程中调用connectDestroyed
  conn->getLoop()->queueInLoop([conn]() { conn->connectDestroyed(); });
  // 从map中移除连接
  {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(conn->name());
  }
}

void TcpServer::handleWrite() { log("Handling write event...", "handleWrite"); }
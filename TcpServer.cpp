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

  // 接受新连接
  Socket *clientsock = new Socket(listensock_->accept(client_addr));

  // 为新连接创建Channel
  Channel *client_channel =
      new Channel(clientsock->getFd(), eventLoop_->getEpoll());

  // // 创建TcpConnection
  // std::shared_ptr<TcpConnection> conn(new TcpConnection(
  //     eventLoop_,
  //     client_addr.getIp() + ":" + std::to_string(client_addr.getPort()),
  //     clientsock->getFd(), server_addr_, client_addr));
  // conn->setConnectionCallback(connectionCallback_);
  // conn->setMessageCallback(messageCallback_);
  // conn->setWriteCompleteCallback(writeCompleteCallback_);
  // conn->setCloseCallback(closeCallback_);
  // conn->connectEstablished();

  // 设置客户端数据处理回调
  // 第一次调用时，clientsock是监听socket，第二次调用时，clientsock是客户端socket
  // 第一次fd=listensock.getFd()，第二次fd=clientsock->getFd()
  // 第一次连接以后，回调就变成下面这个lambda了，以后直接call这个lambda了处理客户端数据
  client_channel->setReadCallback([clientsock, this]() {
    log("Handling client message...", "handleData");
    char buffer[1024];
    ssize_t bytes_read = recv(clientsock->getFd(), buffer, sizeof(buffer), 0);
    if (bytes_read == -1) {
      logError("Error reading from client: " + std::string(strerror(errno)),
               "handleData");
      return;
    }
    if (bytes_read == 0) {
      log("Client disconnected", "handleData");
      clientsock->close();
      this->eventLoop_->getEpoll()->delFd(clientsock->getFd());
      return;
    }
    log("Read " + std::to_string(bytes_read) + " bytes from client",
        "handleData");
    send(clientsock->getFd(), buffer, bytes_read, 0);
    log("Sent " + std::to_string(bytes_read) + " bytes to client",
        "handleData");
  });

  client_channel->useEdgeTrigger(true);
  client_channel->enableReading();

  log("Client connected from " + client_addr.getIp() + ":" +
          std::to_string(client_addr.getPort()),
      "handleNewConnection");
}
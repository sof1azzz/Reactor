#include "TcpConnection.h"
#include <cstring>
#include <iostream>

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             int connfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop), name_(name), state_(kDisconnected),
      socket_(std::make_unique<Socket>(connfd)),
      channel_(std::make_unique<Channel>(connfd, loop_->getEpoll())),
      localAddr_(localAddr), peerAddr_(peerAddr), connectionCallback_(nullptr),
      messageCallback_(nullptr), writeCompleteCallback_(nullptr),
      closeCallback_(nullptr) {
  log("TcpConnection created", "TcpConnection");
}

TcpConnection::~TcpConnection() {}

void TcpConnection::connectEstablished() {
  setState(kConnected);
  channel_->setReadCallback([this]() { handleRead(); });
  // 这里面会调用epoll_ctl(EPOLL_CTL_ADD)，把fd加入到epoll红黑树里面
  channel_->enableReading();
  channel_->useEdgeTrigger(true);
  // 第一次调用时候进入这个回调，这个回调来自main
  // 以后直接调用handleRead()，就是下面的handleRead()回调
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  setState(kDisconnected);
  channel_.reset();
  socket_.reset();
  connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead() {
  log("Handling client message...", "handleData");
  char buffer[1024];
  ssize_t bytes_read = recv(socket_->getFd(), buffer, sizeof(buffer), 0);
  if (bytes_read == -1) {
    logError("Error reading from client: " + std::string(strerror(errno)),
             "handleData");
    return;
  }
  if (bytes_read == 0) {
    log("Client disconnected", "handleData");
    socket_->close();
    loop_->getEpoll()->delFd(socket_->getFd());
    return;
  }
  log("Read " + std::to_string(bytes_read) + " bytes from client",
      "handleData");
  ::send(socket_->getFd(), buffer, bytes_read, 0);
  log("Sent " + std::to_string(bytes_read) + " bytes to client", "handleData");
}

void TcpConnection::handleWrite() {}

void TcpConnection::handleClose() {}

void TcpConnection::handleError() {}
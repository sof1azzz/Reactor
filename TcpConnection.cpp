#include "TcpConnection.h"

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             int connfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop), name_(name), state_(kDisconnected),
      socket_(std::make_unique<Socket>(connfd)), channel_(nullptr),
      localAddr_(localAddr), peerAddr_(peerAddr), connectionCallback_(nullptr),
      messageCallback_(nullptr), writeCompleteCallback_(nullptr),
      closeCallback_(nullptr) {}

TcpConnection::~TcpConnection() {}

void TcpConnection::connectEstablished() {
  setState(kConnected);
  channel_ = std::make_unique<Channel>(socket_->getFd(), loop_->getEpoll());
  channel_->setReadCallback([this]() { handleRead(); });
  channel_->enableReading();
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  setState(kDisconnected);
  channel_.reset();
  socket_.reset();
  connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead() {}

void TcpConnection::handleWrite() {}

void TcpConnection::handleClose() {}

void TcpConnection::handleError() {}
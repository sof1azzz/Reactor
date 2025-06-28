#include "TcpConnection.h"
#include <cstring>

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             int connfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop), name_(name), state_(kDisconnected),
      socket_(std::make_unique<Socket>(connfd)),
      channel_(std::make_unique<Channel>(connfd, loop_->getEpoll())),
      localAddr_(localAddr), peerAddr_(peerAddr), connectionCallback_(nullptr),
      messageCallback_(nullptr), writeCompleteCallback_(nullptr),
      closeCallback_(nullptr), inputBuffer_(), outputBuffer_() {
  log("TcpConnection created", "TcpConnection");
}

TcpConnection::~TcpConnection() {}

void TcpConnection::connectEstablished() {
  setState(kConnected);
  channel_->setReadCallback([this]() { handleRead(); });
  channel_->setCloseCallback([this]() { handleClose(); });
  channel_->setWriteCallback([this]() { handleWrite(); });
  // 这里面会调用epoll_ctl(EPOLL_CTL_ADD)，把fd加入到epoll红黑树里面
  channel_->enableReading();
  channel_->useEdgeTrigger(true);
  // 第一次调用时候进入这个回调，这个回调来自main
  // 以后直接调用handleRead()，就是下面的handleRead()回调
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  if (state_ == kConnected) {
    setState(kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }
  loop_->getEpoll()->delFd(channel_->getFd());
}

void TcpConnection::send(const std::string &buf) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      std::cout << "TcpConnection::send() in loop thread" << std::endl;
      sendInLoop(buf);
    } else {
      std::cout << "TcpConnection::send() not in loop thread" << std::endl;
      loop_->queueInLoop([this, buf]() { send(buf); });
    }
  }
}

void TcpConnection::sendInLoop(const std::string &buf) {
  // if (state_ == kConnected) {
  //   loop_->queueInLoop([this, buf]() { send(buf); });
  // }
  ::send(socket_->getFd(), buf.c_str(), buf.size(), 0);
  log("Sent " + std::to_string(buf.size()) + " bytes to client", "handleData");
}

void TcpConnection::handleRead() {
  ssize_t bytes_read = inputBuffer_.readFd(socket_->getFd());
  if (bytes_read > 0) {
    log("Read " + std::to_string(bytes_read) + " bytes from client",
        "handleData");
    messageCallback_(shared_from_this(), inputBuffer_);
  } else if (bytes_read == 0) {
    handleClose();
  } else {
    handleError();
  }
}

void TcpConnection::handleWrite() {}

void TcpConnection::handleClose() {
  state_ = kDisconnecting;
  channel_->disableAll();

  // 确保在handleClose里面，TcpConnectionPtr不会被释放
  TcpConnectionPtr guardThis(shared_from_this());
  closeCallback_(guardThis);
}

void TcpConnection::handleError() {}
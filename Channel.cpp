#include "Channel.h"
#include <cstring>
#include <experimental/source_location>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

void log(const std::string &message, const std::string &func,
         const std::experimental::source_location &location) {
  std::string filename = location.file_name();
  auto pos = filename.find_last_of("/\\");
  if (pos != std::string::npos)
    filename = filename.substr(pos + 1);

  std::cout << filename << ":" << location.line() << " " << func << " "
            << message << std::endl;
}

void logError(const std::string &message, const std::string &func,
              const std::experimental::source_location &location) {
  log("Error: " + message, func, location);
}

Channel::Channel(int fd, Epoll *epoll)
    : fd_(fd), epoll_(epoll), inEpoll_(false), events_(0), readEvent_(0) {}

Channel::~Channel() {}

int Channel::getFd() const { return fd_; }

void Channel::useEdgeTrigger(bool on) {
  if (on) {
    events_ |= EPOLLET;
  } else {
    events_ &= ~EPOLLET;
  }
}

void Channel::enableReading() {
  events_ |= EPOLLIN;
  epoll_->updateChannel(this);
}

void Channel::enableWriting() {
  events_ |= EPOLLOUT;
  epoll_->updateChannel(this);
}

void Channel::disableWriting() {
  events_ &= ~EPOLLOUT;
  epoll_->updateChannel(this);
}

bool Channel::isWriting() const { return events_ & EPOLLOUT; }

void Channel::setInEpoll(bool on) { inEpoll_ = on; }

void Channel::setReadEvent(bool on) {
  if (on) {
    readEvent_ |= EPOLLIN;
  } else {
    readEvent_ &= ~EPOLLIN;
  }
  epoll_->updateChannel(this);
}

bool Channel::isInEpoll() const { return inEpoll_; }

uint32_t Channel::getEvents() const { return events_; }

uint32_t Channel::getReadEvent() const { return readEvent_; }

void Channel::handleEvent() {
  if (events_ & EPOLLRDHUP) { // 客户端关闭连接
    log("Client disconnected", __func__);
    closeCallback_();
    // 不要在这里关闭fd，调用回调去tcpconnection里面
    // close(fd_);
    // epoll_->delFd(fd_);
    return; // 客户端关闭连接，不需要处理其他事件
  }

  if (events_ & (EPOLLIN | EPOLLPRI)) { // 读事件
    if (readCallback_) {
      readCallback_(); // 调用设置的回调函数，能处理第一次连接和后续数据通信
    }
  } else if (events_ & EPOLLOUT) { // 写事件
    if (writeCallback_) {
      writeCallback_(); // 调用设置的回调函数，能处理第一次连接和后续数据通信
    }
  } else { // 其他事件，忽略
    log("Unknown event", __func__);
  }
}

void Channel::disableAll() {
  events_ = kNoneEvent;
  readEvent_ = kNoneEvent;
  epoll_->updateChannel(this);
}

void Channel::setReadCallback(std::function<void()> callback) { // 设置读回调
  readCallback_ = callback;
}

void Channel::setCloseCallback(std::function<void()> callback) { // 设置关闭回调
  closeCallback_ = callback;
}

void Channel::setWriteCallback(std::function<void()> callback) { // 设置写回调
  writeCallback_ = callback;
}
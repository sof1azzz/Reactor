#pragma once

#include "Channel.h"
#include <sys/epoll.h>
#include <vector>

class Channel;

class Epoll {
public:
  Epoll(int size = kMaxEvents_);
  ~Epoll();

  std::vector<Channel *> loop(int timeout = -1);

  void addFd(int fd, int op);
  void delFd(int fd);
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

private:
  static constexpr int kMaxEvents_ = 1024;
  int epollfd_;
  std::vector<epoll_event> events_;
};
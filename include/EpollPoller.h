#pragma once

#include "Channel.h"
#include "Poller.h"
#include <sys/epoll.h>
#include <vector>

class Channel;

class Epoll : public Poller {
public:
  Epoll();
  ~Epoll();

  void poll(std::vector<Channel *> &activeChannels,
            int timeoutMs = -1) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

private:
  static constexpr int kMaxEvents_ = 1024;
  int epollfd_;
  std::vector<epoll_event> events_;
};
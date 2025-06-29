#include "../include/EpollPoller.h"
#include "../include/Channel.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

Epoll::Epoll() : epollfd_(epoll_create(1)), events_(1024) {
  if (epollfd_ == -1) {
    logError(strerror(errno), __func__);
    exit(EXIT_FAILURE);
  }
}

Epoll::~Epoll() { close(epollfd_); }

void Epoll::updateChannel(Channel *channel) {
  struct epoll_event ev;
  bzero(&ev, sizeof(ev));
  ev.data.ptr = channel;
  ev.events = channel->getEvents();

  // 注册事件
  if (channel->isInEpoll()) {
    if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, channel->getFd(), &ev) == -1) {
      logError(strerror(errno), __func__);
    }
  } else {
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, channel->getFd(), &ev) == -1) {
      logError(strerror(errno), __func__);
    }
    channel->setInEpoll(true);
  }
}

void Epoll::poll(std::vector<Channel *> &activeChannels, int timeoutMs) {
  while (true) {
    int nfds = epoll_wait(epollfd_, events_.data(), events_.size(), -1);
    if (nfds < 0) {
      if (errno == EINTR)
        continue;
      logError(strerror(errno), __func__);
      // 理论上不会执行
      assert(false);
    }
    if (nfds == 0) {
      // timeout 为 -1 时，这里理论上不会执行
      assert(false);
    }
    for (int i = 0; i < nfds; ++i) {
      Channel *ch = (Channel *)events_[i].data.ptr;
      ch->setReadEvent(events_[i].events);
      activeChannels.push_back(ch);
    }
    if (nfds == (int)events_.size()) {
      events_.resize(events_.size() * 2);
    }
    break;
  }
}

void Epoll::removeChannel(Channel *channel) {
  if (channel->isInEpoll()) {
    if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->getFd(), nullptr) == -1) {
      logError(strerror(errno), __func__);
    }
    channel->setInEpoll(false);
  }
}

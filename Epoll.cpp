#include "Epoll.h"
#include "Channel.h"
#include "Socket.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
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

void Epoll::poll(std::vector<Channel *> &channels) {
  while (true) {
    int nfds = epoll_wait(epollfd_, events_.data(), events_.size(), 10);
    if (nfds < 0) {
      if (errno == EINTR)
        continue;
      // error
      break;
    }
    if (nfds == 0) {
      break;
    }
    for (int i = 0; i < nfds; ++i) {
      Channel *ch = (Channel *)events_[i].data.ptr;
      ch->setReadEvent(events_[i].events);
      channels.push_back(ch);
    }
    if (nfds == (int)events_.size()) {
      events_.resize(events_.size() * 2);
    }
    break;
  }
}

void Epoll::delFd(int fd) {
  if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    logError(strerror(errno), __func__);
  }
}

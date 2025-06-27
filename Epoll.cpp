#include "Epoll.h"
#include "Channel.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

Epoll::Epoll(int size) : epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(size) {
  if (size > kMaxEvents_) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << "size is too large" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (epollfd_ == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
}

Epoll::~Epoll() { ::close(epollfd_); }

void Epoll::updateChannel(Channel *channel) {
  const int events = channel->getEvents();
  if (events == Channel::kNoneEvent) {
    if (channel->isInEpoll()) {
      removeChannel(channel);
    }
  } else {                      // 否则是添加或修改
    if (channel->isInEpoll()) { // 已在epoll中，则修改
      epoll_event ev;
      ev.data.ptr = channel;
      ev.events = events;
      epoll_ctl(epollfd_, EPOLL_CTL_MOD, channel->getFd(), &ev);
    } else { // 不在epoll中，则添加
      epoll_event ev;
      ev.data.ptr = channel;
      ev.events = events;
      epoll_ctl(epollfd_, EPOLL_CTL_ADD, channel->getFd(), &ev);
      channel->setInEpoll(true); // **更新状态**
    }
  }

  // 下面是原来的代码,现在要多一个判断，是否要removeChannel
  // epoll_event event;
  // event.data.ptr = channel;
  // event.events = channel->getEvents();

  // int fd = channel->getFd();
  // if (channel->isInEpoll()) {
  //   if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) == -1) {
  //     std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
  //               << strerror(errno) << std::endl;
  //     return;
  //   }
  // } else {
  //   if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) == -1) {
  //     std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
  //               << strerror(errno) << std::endl;
  //     return;
  //   }
  //   channel->setInEpoll(true);
  // }
}

void Epoll::addFd(int fd, int op) {
  std::cerr << "Warning: addFd is deprecated, use updateChannel instead"
            << std::endl;
  /*
  epoll_event event;
  event.data.fd = fd;
  event.events = op;
  if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  */
}

void Epoll::delFd(int fd) { epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr); }

std::vector<Channel *> Epoll::loop(int timeout) {
  std::vector<Channel *> channels;
  memset(events_.data(), 0, sizeof(events_));

  int nfds = epoll_wait(epollfd_, events_.data(), kMaxEvents_, timeout);
  if (nfds == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    return channels;
  }
  if (nfds == 0) {
    std::cout << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << "timeout" << std::endl;
    return channels;
  }
  for (int i = 0; i < nfds; i++) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
    channel->setReadEvent(true);
    channels.push_back(channel);
  }

  return channels;
}

void Epoll::removeChannel(Channel *channel) {
  assert(channel->isInEpoll());
  epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->getFd(), nullptr);
  channel->setInEpoll(false); // **更新状态**
}

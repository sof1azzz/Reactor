#pragma once

#include "Epoll.h"
#include <memory>

class EventLoop {
public:
  EventLoop();
  ~EventLoop();
  void run();
  Epoll *getEpoll() const;

private:
  std::unique_ptr<Epoll> epoll_;
};
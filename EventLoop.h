#pragma once

#include "Epoll.h"

class EventLoop {
public:
  EventLoop();
  ~EventLoop();
  void run();
  Epoll *getEpoll() const;

private:
  Epoll *epoll_;
};
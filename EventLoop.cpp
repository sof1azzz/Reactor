#include "EventLoop.h"

EventLoop::EventLoop() : epoll_(new Epoll()) {}

EventLoop::~EventLoop() { delete epoll_; }

Epoll *EventLoop::getEpoll() const { return epoll_; }

void EventLoop::run() {
  while (true) {
    auto events = epoll_->loop();
    for (auto &event : events) {
      event->handleEvent();
    }
  }
}
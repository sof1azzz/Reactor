#include "EventLoop.h"
#include <memory>

EventLoop::EventLoop() : epoll_(std::make_unique<Epoll>()) {}

EventLoop::~EventLoop() {}

Epoll *EventLoop::getEpoll() const { return epoll_.get(); }

void EventLoop::run() {
  while (true) {
    auto events = epoll_->loop();
    for (auto &event : events) {
      event->handleEvent();
    }
  }
}
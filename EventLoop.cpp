#include "EventLoop.h"
#include "Channel.h"
#include "Callbacks.h"
#include <sys/eventfd.h>
#include <memory>
#include <vector>

EventLoop::EventLoop()
    : epoll_(std::make_unique<Epoll>()), quit_(false), threadId_(std::this_thread::get_id()) {}

EventLoop::~EventLoop() {}

Epoll *EventLoop::getEpoll() const { return epoll_.get(); }

void EventLoop::loop() {
  while (!quit_) {
    std::vector<Channel *> channels;
    epoll_->poll(channels);

    for (auto &channel : channels) {
      channel->handleEvent();
    }
  }
}

void EventLoop::quit() { quit_ = true; }

bool EventLoop::isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

void EventLoop::queueInLoop(std::function<void()> func) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFuncs_.push_back(func);
  }
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::wakeup() {
  
}

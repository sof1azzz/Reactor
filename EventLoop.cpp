#include "EventLoop.h"
#include "Callbacks.h"
#include "Channel.h"
#include <memory>
#include <sys/eventfd.h>
#include <vector>

EventLoop::EventLoop()
    : epoll_(std::make_unique<Epoll>()), quit_(false),
      threadId_(std::this_thread::get_id()) {}

EventLoop::~EventLoop() {}

Epoll *EventLoop::getEpoll() const { return epoll_.get(); }

void EventLoop::loop() {
  while (!quit_) {
    std::vector<Channel *> channels;

    epoll_->poll(channels);

    for (auto &channel : channels) {
      channel->handleEvent();
    }

    // 处理pendingFuncs_ - 每次循环都会执行
    doPendingFunctions();
  }
}

void EventLoop::doPendingFunctions() {
  std::vector<std::function<void()>> functions;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    functions.swap(pendingFuncs_);
  }

  for (const auto &func : functions) {
    func();
  }
}

void EventLoop::quit() { quit_ = true; }

bool EventLoop::isInLoopThread() const {
  return threadId_ == std::this_thread::get_id();
}

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
  // 现在不需要特殊的唤醒机制，因为线程会定期醒来
}

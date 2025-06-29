#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread() : loop_(nullptr) {}

EventLoopThread::~EventLoopThread() {
  if (loop_ != nullptr) {
    loop_->quit();
    if (thread_.joinable()) {
      thread_.join();
    }
  }
}

EventLoop *EventLoopThread::startLoop() {
  thread_ = std::thread(&EventLoopThread::threadFunc, this);

  EventLoop *loop = nullptr;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr) {
      cond_.wait(lock);
    }
    loop = loop_;
  }
  return loop;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }

  loop.loop();
}
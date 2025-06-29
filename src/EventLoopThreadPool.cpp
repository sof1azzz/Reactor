#include "../include/EventLoop.h"
#include "../include/EventLoopThread.h"
#include "../include/EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : baseLoop_(baseLoop), numThreads_(numThreads), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start() {
  for (int i = 0; i < numThreads_; ++i) {
    auto t = std::make_unique<EventLoopThread>();
    loops_.push_back(t->startLoop());
    threads_.push_back(std::move(t));
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  if (loops_.empty()) {
    return baseLoop_;
  }
  EventLoop *loop = loops_[next_];
  next_ = (next_ + 1) % loops_.size();
  return loop;
}
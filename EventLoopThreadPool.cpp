#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <iostream>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : baseLoop_(baseLoop), numThreads_(numThreads), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start() {
  for (int i = 0; i < numThreads_; ++i) {
    auto t = std::make_unique<EventLoopThread>();
    std::cout << "EventLoopThreadPool::start() create thread " << i << std::endl;
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
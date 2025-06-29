#pragma once

#include "EventLoop.h"
#include "EventLoopThread.h"
#include <memory>
#include <vector>

class EventLoopThreadPool {
public:
  EventLoopThreadPool(EventLoop *baseLoop, int numThreads);
  ~EventLoopThreadPool();

  void start();
  EventLoop *getNextLoop();

private:
  EventLoop *baseLoop_; // 主 EventLoop
  int numThreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
};
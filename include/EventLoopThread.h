#pragma once

#include "EventLoop.h"
#include <condition_variable>
#include <mutex>
#include <thread>

class EventLoopThread {
public:
  EventLoopThread();
  ~EventLoopThread();

  EventLoop *startLoop();

private:
  void threadFunc();

  std::thread thread_;
  EventLoop *loop_;
  std::mutex mutex_;
  std::condition_variable cond_;
};
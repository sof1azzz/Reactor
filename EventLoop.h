#pragma once

#include "Epoll.h"
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  Epoll *getEpoll() const;
  bool isInLoopThread() const;
  void queueInLoop(std::function<void()> func);
  void wakeup();

private:
  std::unique_ptr<Epoll> epoll_;
  bool quit_;
  std::vector<std::function<void()>> pendingFuncs_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::thread::id threadId_;
};
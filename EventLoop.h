#pragma once

#include "Epoll.h"
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class Channel;

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
  void handleWakeup(); // for wakeup
  void doPendingFunctions();

  std::unique_ptr<Epoll> epoll_;
  bool quit_;
  std::vector<std::function<void()>> pendingFuncs_;
  std::mutex mutex_;
  const std::thread::id threadId_;
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
};
#pragma once

#include "EpollPoller.h"
#include "Poller.h"
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

  Poller *getPoller() const;
  bool isInLoopThread() const;
  void queueInLoop(std::function<void()> func);
  void wakeup();

private:
  void handleWakeup(); // for wakeup
  void doPendingFunctions();

  std::unique_ptr<Poller> poller_;
  bool quit_;
  std::vector<std::function<void()>> pendingFuncs_;
  std::mutex mutex_;
  const std::thread::id threadId_;
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
};
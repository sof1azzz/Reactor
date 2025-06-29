#include "EventLoop.h"
#include "Channel.h"
#include <memory>
#include <sys/eventfd.h>
#include <vector>

EventLoop::EventLoop()
    : epoll_(std::make_unique<Epoll>()), quit_(false),
      threadId_(std::this_thread::get_id()),
      wakeupFd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      wakeupChannel_(std::make_unique<Channel>(wakeupFd_, epoll_.get())) {
  if (wakeupFd_ < 0) {
    logError("Failed to create wakeupFd", __func__);
  }
  wakeupChannel_->setReadCallback([this]() { handleWakeup(); });
  wakeupChannel_->enableReading();
}

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

// 当 wakeupChannel_ 发生读事件时被调用
void EventLoop::handleWakeup() {
  uint64_t one = 1;
  // 读取 wakeupFd_ 的数据，将计数器清零，否则 epoll 会一直触发
  ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    logError("handleRead() reads " + std::to_string(n) + " bytes instead of 8",
             "EventLoop");
  }
}

// 当其他线程需要唤醒此 EventLoop 时调用
void EventLoop::wakeup() {
  uint64_t one = 1;
  // 向 wakeupFd_ 写入一个 8 字节的数，epoll_wait 将会立即返回
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    logError("wakeup() writes " + std::to_string(n) + " bytes instead of 8",
             "EventLoop");
  }
}

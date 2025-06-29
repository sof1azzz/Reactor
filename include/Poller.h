#pragma once
#include "Channel.h"
#include <memory>
#include <vector>

class Channel;

class Poller {
public:
  virtual ~Poller() = default;

  // 1. 等待 I/O 事件
  virtual void poll(std::vector<Channel *> &activeChannels,
                    int timeoutMs = -1) = 0;

  // 2. 新增 / 修改 fd 的事件
  virtual void updateChannel(Channel *channel) = 0;

  // 3. 删除 fd
  virtual void removeChannel(Channel *channel) = 0;

protected:
  // 供子类持有 EventLoop 指针（必要时回调）
  Poller() = default;
};
#pragma once

#include <functional>
#include <memory>

#include "Buffer.h"

class TcpConnection;

// 连接指针
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

// 连接回调
using TcpConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
// 消息回调
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer &)>;
// 关闭回调
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
// 写完成回调
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;

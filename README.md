# My Reactor Pattern 
# 从notes里面翻译的

## 1. main函数

* 位于 `tcpepoll.cpp`
* 根据命令行参数 `ip` 和 `port` 创建 `TcpServer` 类实例

---

## 2. TcpServer 构造函数

### 2.1 创建下列对象

* `EventLoop`
* `Socket`
* `Channel`
* `InetAddress`
* `std::map` 管理所有 `TcpConnection`

### 2.2 初始化 Socket

* 调用 `bind()` `listen()` 等系统调用

### 2.3 设置 Channel 的回调

* 设置 `readCallback`
* 调用 `enableReading()`

  * 内部调用 `epoll_ctl` 添加 `EPOLLIN`

---

## 3. main 函数继续

* 调用 `TcpServer::setXXXCallback()` 设置上层业务回调
* 调用 `TcpServer::start()` 启动事件循环

---

## 4. TcpServer::start()

* 调用 `EventLoop::run()`
* 微观来看：

  * 调用 `epoll_wait()`
  * 把系统监听到的 fd 封装成 `Channel`
  * 对应调用 `Channel::handleEvent()`

---

## 5. Channel::handleEvent()

### 5.1 如果是 **读事件**：

#### 新连接：

1. 调用 `TcpServer::handleNewConnection()`
2. 通过 `Socket` 获取客户端 fd
3. 创建 `TcpConnection` 对象
4. 加入 `TcpServer` 的 `map`，确保有效引用
5. 设置 `TcpConnection` 的各种 Callback（其中 closeCallback 来自 main）
6. 调用 `TcpConnection::connectEstablished()`

   * 设置 `handleRead()` 为读事件回调
   * 通过 `epoll_ctl` 注册到 epoll 内核 red-black tree

#### 已经连接，执行读:

1. 调用 `TcpConnection::handleRead()`
2. 使用 `Buffer` 和 `Socket` 处理数据
3. 如果需要关闭，调用 closeCallback

#### 关闭 TcpConnection

1. 设置回调时： `TcpServer::removeConnection`
2. 如果观測到关闭：

   * 关闭 fd
   * 调用 `epoll_ctl` DEL 删除
   * 从 `TcpServer` 的 map 删除
3. 当 `TcpConnection` 引用计数为 0 时，自动释放所有内存

---

### 5.2 写事件

* 还未执行，...

---

## 未完成

* ...

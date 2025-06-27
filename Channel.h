#pragma once

#include "Epoll.h"
#include <cstdint>
#include <functional>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

#include <experimental/source_location>

void log(const std::string &message, const std::string &func,
         const std::experimental::source_location &location =
             std::experimental::source_location::current());
void logError(const std::string &message, const std::string &func,
              const std::experimental::source_location &location =
                  std::experimental::source_location::current());

class Epoll;
class Socket;
class InetAddress;

class Channel {
public:
  Channel(int fd, Epoll *epoll);
  ~Channel();

  int getFd() const;
  void useEdgeTrigger(bool on);
  void enableReading();
  void setInEpoll(bool on);
  void setReadEvent(bool on);
  bool isInEpoll() const;
  uint32_t getEvents() const;
  uint32_t getReadEvent() const;
  void handleEvent();
  void setReadCallback(std::function<void()> callback);
  void setCloseCallback(std::function<void()> callback);

  void disableAll();

  static const int kNoneEvent = 0;
  static const int kReadEvent = EPOLLIN | EPOLLPRI;
  static const int kWriteEvent = EPOLLOUT;

private:
  int fd_;
  Epoll *epoll_;
  bool inEpoll_;
  uint32_t events_;
  uint32_t readEvent_;
  std::function<void()> readCallback_;
  std::function<void()> closeCallback_;
};
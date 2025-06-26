#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "InetAddress.h"

int createNonblockingSocket();

class Socket {
public:
  Socket(int fd);
  ~Socket();
  int getFd() const;
  void setReuseAddr(bool on);
  void setReusePort(bool on);
  void setTcpNoDelay(bool on);
  void setKeepAlive(bool on);

  void bind(const InetAddress &addr);
  void listen(int backlog = 1024);
  int accept(InetAddress &addr) const;
  void close();

private:
  int fd_;
};
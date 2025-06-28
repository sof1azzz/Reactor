#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "InetAddress.h"

int createNonblockingSocket();

class Socket {
public:
  Socket(int fd);
  Socket();
  ~Socket();
  int getFd() const;
  void bind(const InetAddress &addr);
  void listen();
  int accept(InetAddress &addr);
  void setReuseAddr(bool on);
  void setReusePort(bool on);
  void setTcpNoDelay(bool on);
  void setKeepAlive(bool on);

  static struct sockaddr_in getLocalAddr(int sockfd);
  static struct sockaddr_in getPeerAddr(int sockfd);

private:
  const int fd_;
};
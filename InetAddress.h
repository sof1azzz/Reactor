#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class InetAddress {
public:
  InetAddress(const std::string &ip, int port);
  InetAddress(const struct sockaddr_in &addr);
  InetAddress();
  ~InetAddress();

  std::string getIp() const;
  int getPort() const;
  const struct sockaddr *getSockAddr() const;
  void setSockAddr(const struct sockaddr *addr);

private:
  struct sockaddr_in addr_;
};

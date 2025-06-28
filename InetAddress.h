#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class InetAddress {
public:
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);
  InetAddress(const std::string &ip, uint16_t port);
  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}
  ~InetAddress();

  std::string getIp() const;
  uint16_t getPort() const;
  const struct sockaddr *getAddr() const;
  void setAddr(const struct sockaddr_in &addr) { addr_ = addr; }

private:
  struct sockaddr_in addr_;
};

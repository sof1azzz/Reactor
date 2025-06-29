#include "InetAddress.h"
#include <cstring>

InetAddress::InetAddress(uint16_t port, bool loopbackOnly) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
  addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
    // Here you should ideally log an error
  }
  addr_.sin_port = htons(port);
}

std::string InetAddress::getIp() const {
  char buf[64] = "";
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
  return buf;
}

uint16_t InetAddress::getPort() const { return ntohs(addr_.sin_port); }

const struct sockaddr *InetAddress::getAddr() const {
  return static_cast<const struct sockaddr *>(
      static_cast<const void *>(&addr_));
}

InetAddress::~InetAddress() {}
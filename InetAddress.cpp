#include "InetAddress.h"

InetAddress::InetAddress(const std::string &ip, int port) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

InetAddress::InetAddress() {
}

InetAddress::~InetAddress() {}

std::string InetAddress::getIp() const {
  return inet_ntoa(addr_.sin_addr);
}

int InetAddress::getPort() const {
  return ntohs(addr_.sin_port);
}

const struct sockaddr *InetAddress::getSockAddr() const {
  return reinterpret_cast<const struct sockaddr *>(&addr_);
}

void InetAddress::setSockAddr(const struct sockaddr *addr) {
  addr_ = *reinterpret_cast<const struct sockaddr_in *>(addr);
}
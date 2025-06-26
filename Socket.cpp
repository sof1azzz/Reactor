#include "Socket.h"
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/tcp.h>
#include <sys/socket.h>

int createNonblockingSocket() {
  int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (fd == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  return fd;
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() { ::close(fd_); }

int Socket::getFd() const { return fd_; }

void Socket::setReuseAddr(bool on) {
  int opt = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::setReusePort(bool on) {
  int opt = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void Socket::setTcpNoDelay(bool on) {
  int opt = on ? 1 : 0;
  ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

void Socket::setKeepAlive(bool on) {
  int opt = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}

void Socket::bind(const InetAddress &addr) {
  if (::bind(fd_, addr.getSockAddr(), sizeof(addr)) == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Socket::listen(int backlog) {
  if (::listen(fd_, backlog) == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
}

int Socket::accept(InetAddress &addr) const {
  sockaddr_in peer_addr;
  socklen_t peer_addr_len = sizeof(peer_addr);
  int clientsock =
      ::accept4(fd_, reinterpret_cast<struct sockaddr *>(&peer_addr),
                 &peer_addr_len,
                 SOCK_NONBLOCK);
  if (clientsock == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  addr.setSockAddr(reinterpret_cast<struct sockaddr *>(&peer_addr));
  return clientsock;
}

void Socket::close() { ::close(fd_); }
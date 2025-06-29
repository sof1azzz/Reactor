#include "Socket.h"
#include "InetAddress.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
Socket::Socket() : fd_(createNonblockingSocket()) {}

Socket::~Socket() { ::close(fd_); }

int Socket::getFd() const { return fd_; }

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
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
  if (::bind(fd_, addr.getAddr(), sizeof(struct sockaddr_in)) == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Socket::listen() {
  if (::listen(fd_, SOMAXCONN) == -1) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
}

int Socket::accept(InetAddress &addr) {
  struct sockaddr_in client_addr;
  socklen_t len = sizeof(client_addr);
  int connfd = ::accept4(fd_, (struct sockaddr *)&client_addr, &len,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0) {
    addr.setAddr(client_addr);
  }
  return connfd;
}

struct sockaddr_in Socket::getLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof(localaddr);
  if (::getsockname(sockfd, (struct sockaddr *)&localaddr, &addrlen) < 0) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
  }
  return localaddr;
  }

struct sockaddr_in Socket::getPeerAddr(int sockfd) {
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = sizeof(peeraddr);
  if (::getpeername(sockfd, (struct sockaddr *)&peeraddr, &addrlen) < 0) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__ << " "
              << strerror(errno) << std::endl;
  }
  return peeraddr;
}
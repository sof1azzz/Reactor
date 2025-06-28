#include <arpa/inet.h>
#include <chrono>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

class ctcp_client {
public:
  int m_clientfd;
  std::string m_ip;
  unsigned short m_port;

  ctcp_client() : m_clientfd(-1) {}
  ~ctcp_client() { close(); }

  bool connect(const std::string &in_ip, unsigned short in_port) {
    if (m_clientfd != -1) {
      return false;
    }

    m_ip = in_ip;
    m_port = in_port;

    if ((m_clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      std::cerr << "Error in ctcp_client::connect: Failed to create socket"
                << std::endl;
      return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_port);

    struct hostent *h;
    if ((h = gethostbyname(m_ip.c_str())) == NULL) {
      std::cerr << "Error in ctcp_client::connect: Failed to get host by name"
                << std::endl;
      close();
      m_clientfd = -1;
      return false;
    }
    memcpy(&server_addr.sin_addr, h->h_addr_list[0], h->h_length);

    if (::connect(m_clientfd, (struct sockaddr *)&server_addr,
                  sizeof(server_addr)) < 0) {
      std::cerr << "Error in ctcp_client::connect: Failed to connect to server"
                << std::endl;
      close();
      m_clientfd = -1;
      return false;
    }
    return true;
  }

  bool connect_nonblock(const std::string &in_ip, unsigned short in_port) {
    if (m_clientfd != -1) {
      return false;
    }

    m_ip = in_ip;
    m_port = in_port;

    if ((m_clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      std::cerr
          << "Error in ctcp_client::connect_nonblock: Failed to create socket"
          << std::endl;
      return false;
    }

    if (setnonblock(m_clientfd) == -1) {
      std::cerr << "Error in ctcp_client::connect_nonblock: Failed to set "
                   "non-blocking mode"
                << std::endl;
      close();
      return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(in_port);

    struct hostent *h;
    if ((h = gethostbyname(in_ip.c_str())) == NULL) {
      std::cerr << "Error in ctcp_client::connect_nonblock: Failed to get host "
                   "by name"
                << std::endl;
      close();
      return false;
    }
    memcpy(&server_addr.sin_addr, h->h_addr_list[0], h->h_length);

    int result = ::connect(m_clientfd, (struct sockaddr *)&server_addr,
                           sizeof(server_addr));
    if (result < 0) {
      if (errno == EINPROGRESS) {
        return wait_for_connection();
      } else {
        std::cerr << "Error in ctcp_client::connect_nonblock: Failed to "
                     "connect to server"
                  << std::endl;
        close();
        return false;
      }
    }
    return true;
  }

  bool wait_for_connection() {
    fd_set write_fds;
    struct timeval timeout;

    FD_ZERO(&write_fds);
    FD_SET(m_clientfd, &write_fds);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int result = select(m_clientfd + 1, NULL, &write_fds, NULL, &timeout);
    if (result <= 0) {
      std::cerr << "Error in ctcp_client::wait_for_connection: Connection "
                   "timeout or error"
                << std::endl;
      close();
      return false;
    }

    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(m_clientfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
      std::cerr << "Error in ctcp_client::wait_for_connection: Failed to get "
                   "socket error"
                << std::endl;
      close();
      return false;
    }

    if (error != 0) {
      std::cerr << "Error in ctcp_client::wait_for_connection: Connection "
                   "failed with error "
                << error << std::endl;
      close();
      return false;
    }

    std::cout << "Connected successfully!" << std::endl;
    return true;
  }

  bool send(const std::string &in_message) {
    if (m_clientfd == -1) {
      return false;
    }

    if (::send(m_clientfd, in_message.c_str(), in_message.size(), 0) <= 0) {
      return false;
    }
    return true;
  }

  bool recv(std::string &out_message) {
    if (m_clientfd == -1) {
      return false;
    }

    out_message.clear();
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int n = ::recv(m_clientfd, buffer, sizeof(buffer), 0);
    if (n < 0) {
      std::cerr << "Error in ctcp_client::recv: Failed to receive message"
                << std::endl;
      return false;
    } else if (n == 0) {
      std::cout << "Server disconnected" << std::endl;
      return false;
    }
    out_message.append(buffer, n);
    return true;
  }

  bool close() {
    if (m_clientfd == -1) {
      return false;
    }
    ::close(m_clientfd);
    m_clientfd = -1;
    return true;
  }

  int setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
      return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  }
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  ctcp_client client;

  // 使用阻塞连接
  if (!client.connect(argv[1], atoi(argv[2]))) {
    std::cerr << "Error in main: Failed to connect to server" << std::endl;
    return 1;
  }

  while (true) {
    if (!client.send("Hello, server!")) {
      std::cerr << "Error in main: Failed to send message" << std::endl;
      break;
    }

    std::cout << "send message: Hello, server!" << std::endl;

    std::string message;
    if (!client.recv(message)) {
      std::cerr << "Error in main: Failed to receive message" << std::endl;
      break;
    }
    std::cout << "Received message: " << message << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}

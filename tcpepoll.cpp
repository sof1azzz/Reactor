#include "TcpServer.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    logError("Usage: " + std::string(argv[0]) + " <ip> <port>", "main");
    return 1;
  }
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  TcpServer tcpServer(ip, port);
  tcpServer.start();
  return 0;
}

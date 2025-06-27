#pragma once

#include <algorithm> // for std::swap
#include <cstddef>   // for size_t
#include <string>
#include <sys/uio.h> // for readv
#include <vector>

class Buffer {
public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize);
  ~Buffer();

  size_t readableBytes() const;
  size_t writableBytes() const;
  size_t prependableBytes() const;

  const char *peek() const;
  void retrieve(size_t len);
  void retrieveAll();
  std::string retrieveAllAsString();
  std::string retrieveAsString(size_t len);

  void append(const char *data, size_t len);
  void ensureWritableBytes(size_t len);
  char *beginWrite();
  const char *beginWrite() const;
  void hasWritten(size_t len);

  const char *findCRLF() const;

  ssize_t readFd(int fd);

private:
  char *begin();
  const char *begin() const;
  void makeSpace(size_t len);

private:
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
};
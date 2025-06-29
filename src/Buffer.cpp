#include "../include/Buffer.h"

/**
 * @brief 构造函数
 * @param initialSize 初始大小
 */
Buffer::Buffer(size_t initialSize)
    : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend) {}

Buffer::~Buffer() { buffer_.clear(); }

/**
 * @brief 可读字节数
 * @return
 */
size_t Buffer::readableBytes() const { return writerIndex_ - readerIndex_; }

/**
 * @brief 可写字节数
 * @return
 */
size_t Buffer::writableBytes() const { return buffer_.size() - writerIndex_; }

/**
 * @brief 预留字节数
 * @return
 */
size_t Buffer::prependableBytes() const { return readerIndex_; }

/**
 * @brief 返回可读数据的起始地址
 * @return
 */
const char *Buffer::peek() const { return begin() + readerIndex_; }

/**
 * @brief 移动readerIndex_
 * @param len
 */
void Buffer::retrieve(size_t len) {
  if (len < readableBytes()) {
    readerIndex_ += len;
  } else {
    retrieveAll();
  }
}

/**
 * @brief 重置缓冲区
 */
void Buffer::retrieveAll() {
  readerIndex_ = kCheapPrepend;
  writerIndex_ = kCheapPrepend;
}

/**
 * @brief 将可读数据转换为string
 * @return
 */
std::string Buffer::retrieveAllAsString() {
  return retrieveAsString(readableBytes());
}

/**
 * @brief 将指定长度的数据转换为string
 * @param len
 * @return
 */
std::string Buffer::retrieveAsString(size_t len) {
  std::string result(peek(), len);
  retrieve(len);
  return result;
}

/**
 * @brief 添加数据
 * @param data
 * @param len
 */
void Buffer::append(const char *data, size_t len) {
  ensureWritableBytes(len);
  std::copy(data, data + len, beginWrite());
  writerIndex_ += len;
}

/**
 * @brief 确保有足够的可写空间
 * @param len
 */
void Buffer::ensureWritableBytes(size_t len) {
  if (writableBytes() < len) {
    makeSpace(len);
  }
}

/**
 * @brief 返回可写缓冲区的起始地址
 * @return
 */
char *Buffer::beginWrite() { return begin() + writerIndex_; }

/**
 * @brief 返回可写缓冲区的起始地址
 * @return
 */
const char *Buffer::beginWrite() const { return begin() + writerIndex_; }

/**
 * @brief 移动writerIndex_
 * @param len
 */
void Buffer::hasWritten(size_t len) { writerIndex_ += len; }

/**
 * @brief 查找CRLF
 * @return
 */
const char *Buffer::findCRLF() const {
  const char *crlf = std::search(peek(), beginWrite(), "\r\n", "\r\n" + 2);
  return crlf == beginWrite() ? nullptr : crlf;
}

/**
 * @brief 从fd读取数据
 * @param fd
 * @return
 */
ssize_t Buffer::readFd(int fd) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = beginWrite();
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);
  const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
  const ssize_t n = ::readv(fd, vec, iovcnt);
  if (n < 0) {
    return -1;
  } else if (static_cast<size_t>(n) <= writable) {
    writerIndex_ += n;
  } else {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  return n;
}

/**
 * @brief 返回缓冲区的起始地址
 * @return
 */
char *Buffer::begin() { return &*buffer_.begin(); }

/**
 * @brief 返回缓冲区的起始地址
 * @return
 */
const char *Buffer::begin() const { return &*buffer_.begin(); }

/**
 * @brief 腾出空间
 * @param len
 */
void Buffer::makeSpace(size_t len) {
  if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
    buffer_.resize(writerIndex_ + len);
  } else {
    size_t readable = readableBytes();
    std::copy(begin() + readerIndex_, begin() + writerIndex_,
              begin() + kCheapPrepend);
    readerIndex_ = kCheapPrepend;
    writerIndex_ = readerIndex_ + readable;
  }
}
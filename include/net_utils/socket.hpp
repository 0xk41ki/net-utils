#pragma once

#include "net_utils/errors.hpp"
#include "net_utils/socket_addr.hpp"
#include <cstddef>
namespace net_utils {
class RawSocket {
public:
  static NetResult<RawSocket, int> create(int domain, int type,
                                          int protocol) noexcept;
  NetResult<void, int> set_flags(int flags) noexcept;
  NetResult<int, int> get_flags() noexcept;
  NetResult<void, int> connect(const SocketAddr &addr) noexcept;
  NetResult<void, int> bind(const SocketAddr &addr) noexcept;
  NetResult<long, int> read(char *buf, std::size_t len) noexcept;
  NetResult<long, int> write(char *buf, std::size_t len) noexcept;
  NetResult<void, int> close() noexcept;
  inline int fd() { return fd_; }
  RawSocket(const RawSocket &) = delete;
  RawSocket &operator=(const RawSocket &) = delete;
  RawSocket(RawSocket &&other) noexcept
      : fd_(other.fd_), is_closed_(other.is_closed_) {
    other.is_closed_ = true; // prevent the moved-from dtor from closing fd_
  }
  RawSocket &operator=(RawSocket &&other) noexcept {
    if (this != &other) {
      auto _ = close();
      fd_ = other.fd_;
      is_closed_ = other.is_closed_;
      other.is_closed_ = true;
    }
    return *this;
  }
  ~RawSocket();

private:
  RawSocket(int fd) : fd_(fd), is_closed_(false) {};
  int fd_;
  bool is_closed_;
};
} // namespace net_utils

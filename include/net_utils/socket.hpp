#pragma once

#include "net_utils/errors.hpp"
#include "net_utils/socket_addr.hpp"
#include <cstdint>
namespace net_utils {
class RawSocket {
public:
  static NetResult<RawSocket, std::uint64_t> create(int domain, int type,
                                                    int protocol) noexcept;
  NetResult<void, std::uint64_t> set_flags(int flags) noexcept;
  NetResult<int, std::uint64_t> get_flags() noexcept;
  NetResult<void, std::uint64_t> connect(const SocketAddr &addr) noexcept;
  NetResult<void, std::uint64_t> bind(const SocketAddr &addr) noexcept;
  NetResult<long, std::uint64_t> read(char *buf, std::size_t len) noexcept;
  NetResult<long, std::uint64_t> write(char *buf, std::size_t len) noexcept;
  NetResult<void, std::uint64_t> close() noexcept;
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

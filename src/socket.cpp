#include <cerrno>
#include <fcntl.h>
#include <net_utils/errors.hpp>
#include <net_utils/socket.hpp>
#include <net_utils/socket_addr.hpp>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace net_utils;

NetResult<RawSocket, std::uint64_t> RawSocket::create(int domain, int type,
                                                      int protocol) noexcept {
  int fd = ::socket(domain, type, protocol);
  if (fd < 0) {
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    static_cast<std::uint64_t>(errno)));
  }

  return RawSocket(fd);
};

RawSocket::~RawSocket() { auto _ = close(); }

NetResult<void, std::uint64_t> RawSocket::close() noexcept {
  if (!is_closed_) {
    if (::close(fd_) != 0) {
      return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                      static_cast<std::uint64_t>(errno)));
    }
    is_closed_ = true;
    return {};
  }

  return {};
}

NetResult<void, std::uint64_t>
RawSocket::connect(const SocketAddr &addr) noexcept {
  if (::connect(fd_, addr.data(), addr.size()) < 0) {
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    static_cast<std::uint64_t>(errno)));
  }
  return {};
}

NetResult<void, std::uint64_t>
RawSocket::bind(const SocketAddr &addr) noexcept {
  if (::bind(fd_, addr.data(), addr.size()) < 0) {
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    static_cast<std::uint64_t>(errno)));
  }
  return {};
}

NetResult<long, std::uint64_t> RawSocket::read(char *buf,
                                               std::size_t len) noexcept {
  long n = ::recv(fd_, buf, len, 0);
  if (n < 0) {
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    static_cast<std::uint64_t>(errno)));
  }
  if (n == 0) {
    return std::unexpected(
        NetError<std::uint64_t>(NetErrorCode::ConnectionClosed));
  }
  return n;
}
NetResult<long, std::uint64_t> RawSocket::write(char *buf,
                                                std::size_t len) noexcept {
  long n = ::send(fd_, buf, len, 0);
  if (n < 0) {
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    static_cast<std::uint64_t>(errno)));
  }
  if (n == 0) {
    return std::unexpected(
        NetError<std::uint64_t>(NetErrorCode::ConnectionClosed));
  }
  return n;
}

NetResult<int, std::uint64_t> RawSocket::get_flags() noexcept {
  int res = fcntl(fd_, F_GETFL, 0);
  if (res < 0) {
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    static_cast<std::uint64_t>(errno)));
  }
  return res;
}

NetResult<void, std::uint64_t> RawSocket::set_flags(int flags) noexcept {
  if (fcntl(fd_, F_SETFL, flags) < 0) {
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    static_cast<std::uint64_t>(errno)));
  }
  return {};
}

#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

namespace net_utils {

class SocketAddr {
public:
  SocketAddr() = default;
  SocketAddr(const struct sockaddr *sa, socklen_t len) : len_(len) {
    std::memcpy(&storage_, sa, len);
  }

  const struct sockaddr *data() const noexcept {
    return reinterpret_cast<const struct sockaddr *>(&storage_);
  }
  socklen_t size() const noexcept { return len_; }
  int family() const noexcept { return storage_.ss_family; }

  std::string to_string() const {
    constexpr size_t BUFLEN = INET6_ADDRSTRLEN;
    char buf[BUFLEN];
    if (family() == AF_INET) {
      inet_ntop(AF_INET,
                &reinterpret_cast<const sockaddr_in *>(&storage_)->sin_addr,
                buf, BUFLEN);
    } else {
      inet_ntop(AF_INET6,
                &reinterpret_cast<const sockaddr_in6 *>(&storage_)->sin6_addr,
                buf, BUFLEN);
    }
    return std::string(buf);
  }

private:
  struct sockaddr_storage storage_{};
  socklen_t len_{0};
};

} // namespace net_utils

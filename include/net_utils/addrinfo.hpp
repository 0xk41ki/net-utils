#pragma once

#include <net_utils/socket_addr.hpp>
#include <string>

namespace net_utils {
struct AddrInfo {
  int family;
  int socktype;
  int protocol;
  SocketAddr address;

  std::string to_string() const { return address.to_string(); }
};

} // namespace net_utils

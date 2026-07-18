#include <cstdint>
#include <cstring>
#include <net_utils/dns.hpp>
#include <net_utils/errors.hpp>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

net_utils::NetResult<std::size_t, std::uint64_t>
net_utils::DnsResolver::resolve() {
  struct addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *res = nullptr;
  int rc = getaddrinfo(host_.c_str(), port_.c_str(), &hints, &res);

  if (rc != 0) {
    return std::unexpected(NetError(NetErrorCode::AddressResolutionFailed,
                                    static_cast<uint64_t>(rc)));
  }
  addrs_.clear();
  for (const auto *ai = res; ai != nullptr; ai = ai->ai_next) {
    addrs_.push_back(AddrInfo{ai->ai_family, ai->ai_socktype, ai->ai_protocol,
                              SocketAddr(ai->ai_addr, ai->ai_addrlen)});
  }
  freeaddrinfo(res);

  return addrs_.size();
};

const net_utils::AddrInfo &
net_utils::DnsResolver::get_result_at(std::size_t idx) const {
  return addrs_.at(idx);
}

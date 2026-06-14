#pragma once
#include <arpa/inet.h>
#include <cstddef>
#include <net_utils/errors.hpp>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <vector>
namespace net_utils {
struct AddrInfo {
  int family_;
  int socktype_;
  int protocol_;
  socklen_t addr_len_;
  struct sockaddr *addr_;

public:
  AddrInfo(int family, int socktype, int protocol, socklen_t addr_len,
           struct sockaddr *addr);
  ~AddrInfo();
  AddrInfo(const AddrInfo &);
  AddrInfo &operator=(const AddrInfo &);
  AddrInfo(AddrInfo &&) noexcept;
  AddrInfo &operator=(AddrInfo &&) noexcept;
  inline std::string to_string() const {
    constexpr const size_t BUFLEN = INET6_ADDRSTRLEN;
    char buf[BUFLEN];
    if (family_ == AF_INET) {
      inet_ntop(family_, &reinterpret_cast<sockaddr_in *>(addr_)->sin_addr, buf,
                BUFLEN);
    } else {
      inet_ntop(family_, &reinterpret_cast<sockaddr_in6 *>(addr_)->sin6_addr,
                buf, BUFLEN);
    }
    return std::string(buf);
  };
  int family() const noexcept { return family_; }
  int socktype() const noexcept { return socktype_; }
  int protocol() const noexcept { return protocol_; }
  socklen_t addr_len() const noexcept { return addr_len_; }
  const struct sockaddr *addr() const noexcept { return addr_; }
};

class DnsResolver {
public:
  DnsResolver(std::string host, std::string port)
      : host_(std::move(host)), port_(std::move(port)) {};
  net_utils::NetResult<int, int> resolve();
  std::size_t num_results() const noexcept;
  const AddrInfo &get_result_at(std::size_t) const noexcept;

private:
  std::string host_;
  std::string port_;
  std::vector<AddrInfo> addrs_;
}; // class DnsResolver

} // namespace net_utils

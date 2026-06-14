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
    char *buf = new char[INET6_ADDRSTRLEN];
    inet_ntop(family_, &addr_, buf, INET6_ADDRSTRLEN);
    return std::string(buf, buf + INET6_ADDRSTRLEN);
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

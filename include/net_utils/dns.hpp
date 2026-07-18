#pragma once
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <net_utils/addrinfo.hpp>
#include <net_utils/errors.hpp>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <vector>
namespace net_utils {

class DnsResolver {
public:
  DnsResolver(std::string host, std::string port)
      : host_(std::move(host)), port_(std::move(port)) {};
  net_utils::NetResult<std::size_t, std::uint64_t> resolve();
  const AddrInfo &get_result_at(std::size_t) const;

private:
  std::string host_;
  std::string port_;
  std::vector<AddrInfo> addrs_;
}; // class DnsResolver

} // namespace net_utils

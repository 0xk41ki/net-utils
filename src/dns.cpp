#include <cstring>
#include <net_utils/dns.hpp>
#include <net_utils/errors.hpp>
#include <netdb.h>
#include <new>
#include <ostream>
#include <sys/socket.h>
#include <vector>

// net_utils::DnsResolver::DnsResolver() {
//
// };

net_utils::NetResult<int, int> net_utils::DnsResolver::resolve() {
  struct addrinfo hints{};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *res = nullptr;
  int rc = getaddrinfo(host_.c_str(), port_.c_str(), &hints, &res);

  if (rc < 0) {
    // some failure happened. TODO: handle this
    return std::unexpected(NetError(NetErrorCode::AddressResolutionFailed, rc));
  }

  for (const auto *ai = res; ai != nullptr; ai = ai->ai_next) {
    addrs_.push_back(AddrInfo(ai->ai_family, ai->ai_socktype, ai->ai_protocol,
                              ai->ai_addrlen, ai->ai_addr));
  }
  freeaddrinfo(res);

  return 0;
};

std::size_t net_utils::DnsResolver::num_results() const noexcept {
  return addrs_.size();
};

const net_utils::AddrInfo &
net_utils::DnsResolver::get_result_at(std::size_t idx) const noexcept {
  return addrs_.at(idx);
}

net_utils::AddrInfo::AddrInfo(int family, int socktype, int protocol,
                              socklen_t addr_len, struct sockaddr *addr)
    : family_(family), socktype_(socktype), protocol_(protocol),
      addr_len_(addr_len), addr_(nullptr) {
  addr_ = static_cast<struct sockaddr *>(::operator new(addr_len));
  memcpy(addr_, addr, addr_len);
};

net_utils::AddrInfo::~AddrInfo() { ::operator delete(addr_); };

net_utils::AddrInfo::AddrInfo(const AddrInfo &orig)
    : family_(orig.family_), socktype_(orig.socktype_),
      protocol_(orig.protocol_), addr_len_(orig.addr_len_) {
  addr_ = static_cast<struct sockaddr *>(::operator new(addr_len_));
  memcpy(addr_, orig.addr_, addr_len_);
}

net_utils::AddrInfo &net_utils::AddrInfo::operator=(const AddrInfo &orig) {
  if (&orig != this) {
    family_ = orig.family_;
    socktype_ = orig.socktype_;
    protocol_ = orig.protocol_;
    addr_len_ = orig.addr_len_;
    addr_ = static_cast<struct sockaddr *>(::operator new(addr_len_));
    memcpy(addr_, orig.addr_, addr_len_);
  }
  return *this;
}

net_utils::AddrInfo::AddrInfo(AddrInfo &&orig) noexcept
    : family_(orig.family_), socktype_(orig.socktype_),
      protocol_(orig.protocol_), addr_len_(orig.addr_len_), addr_(orig.addr_) {
  orig.addr_ = nullptr;
  orig.addr_len_ = 0;
}

net_utils::AddrInfo &net_utils::AddrInfo::operator=(AddrInfo &&orig) noexcept {
  if (&orig != this) {
    family_ = orig.family_;
    socktype_ = orig.socktype_;
    protocol_ = orig.protocol_;
    addr_len_ = orig.addr_len_;
    addr_ = orig.addr_;
    orig.addr_ = nullptr;
    orig.addr_len_ = 0;
  }
  return *this;
}

std::ostream &operator<<(std::ostream &os, net_utils::AddrInfo &dns) {
  return os << dns.to_string();
}

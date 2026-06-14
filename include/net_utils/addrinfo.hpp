#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <sys/socket.h>

namespace net_utils {
struct AddrInfo {
  int family_;
  int socktype_;
  int protocol_;
  socklen_t addr_len_;
  struct sockaddr *addr_;

public:
  AddrInfo(int family, int socktype, int protocol, socklen_t addr_len,
           struct sockaddr *addr)
      : family_(family), socktype_(socktype), protocol_(protocol),
        addr_len_(addr_len), addr_(nullptr) {
    addr_ = static_cast<struct sockaddr *>(::operator new(addr_len_));
    memcpy(addr_, addr, addr_len);
  };
  ~AddrInfo() { ::operator delete(addr_); };
  AddrInfo(const AddrInfo &orig)
      : family_(orig.family_), socktype_(orig.socktype_),
        protocol_(orig.protocol_), addr_len_(orig.addr_len_) {
    addr_ = static_cast<struct sockaddr *>(::operator new(addr_len_));
    memcpy(addr_, orig.addr_, addr_len_);
  };
  AddrInfo &operator=(const AddrInfo &orig) {
    if (&orig != this) {
      family_ = orig.family_;
      socktype_ = orig.socktype_;
      protocol_ = orig.protocol_;
      addr_len_ = orig.addr_len_;
      ::operator delete(addr_);
      addr_ = static_cast<struct sockaddr *>(::operator new(addr_len_));
      memcpy(addr_, orig.addr_, addr_len_);
    }
    return *this;
  };
  AddrInfo(AddrInfo &&orig) noexcept
      : family_(orig.family_), socktype_(orig.socktype_),
        protocol_(orig.protocol_), addr_len_(orig.addr_len_),
        addr_(orig.addr_) {
    orig.addr_ = nullptr;
    orig.addr_len_ = 0;
  };
  AddrInfo &operator=(AddrInfo &&orig) noexcept {
    if (&orig != this) {
      family_ = orig.family_;
      socktype_ = orig.socktype_;
      protocol_ = orig.protocol_;
      addr_len_ = orig.addr_len_;
      ::operator delete(addr_);
      addr_ = orig.addr_;
      orig.addr_ = nullptr;
      orig.addr_len_ = 0;
    }
    return *this;
  };
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
} // namespace net_utils

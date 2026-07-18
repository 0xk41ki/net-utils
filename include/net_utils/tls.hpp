#pragma once

#include "net_utils/errors.hpp"
#include "net_utils/socket.hpp"
#include "openssl/ssl.h"
#include <cstdint>
#include <functional>
namespace net_utils {
class OpenSSLCtx {
public:
  static net_utils::NetResult<std::reference_wrapper<OpenSSLCtx>, std::uint64_t>
  instance();
  inline ::SSL_CTX *ptr() { return ctx_; };

  OpenSSLCtx(const OpenSSLCtx &) = delete;
  OpenSSLCtx &operator=(const OpenSSLCtx &) = delete;
  OpenSSLCtx(OpenSSLCtx &&) = delete;
  OpenSSLCtx &operator=(OpenSSLCtx &&) = delete;

private:
  OpenSSLCtx();
  ~OpenSSLCtx();
  bool is_ok_ = false;
  net_utils::NetError<std::uint64_t> init_error_{
      net_utils::NetErrorCode::InternalSSLError, 0};
  ::SSL_CTX *ctx_ = nullptr;
}; // class OpenSSLCtx

class TlsSocket {
public:
  static NetResult<TlsSocket, std::uint64_t>
  create(const std::string &hostname) noexcept;
  NetResult<void, std::uint64_t> set_flags(int flags) noexcept {
    return underlying_.set_flags(flags);
  };
  NetResult<int, std::uint64_t> get_flags() noexcept {
    return underlying_.get_flags();
  };
  NetResult<void, std::uint64_t> connect(const SocketAddr &addr) noexcept;
  NetResult<void, std::uint64_t> bind(const SocketAddr &addr) noexcept {
    return underlying_.bind(addr);
  };
  NetResult<std::size_t, std::uint64_t> read(char *buf,
                                             std::size_t len) noexcept;
  NetResult<std::size_t, std::uint64_t> write(const char *buf,
                                              std::size_t len) noexcept;
  NetResult<void, std::uint64_t> close() noexcept;
  static std::string error_decode_helper(std::uint64_t error_code) noexcept;
  TlsSocket(const TlsSocket &) = delete;
  TlsSocket &operator=(const TlsSocket &) = delete;
  TlsSocket(TlsSocket &&other) noexcept
      : underlying_(std::move(other.underlying_)), ssl_(other.ssl_),
        is_closed_(other.is_closed_) {
    other.is_closed_ = true; // prevent the moved-from dtor from closing fd_
  }
  TlsSocket &operator=(TlsSocket &&other) noexcept {
    if (this != &other) {
      auto _ = close();
      underlying_ = std::move(other.underlying_);
      is_closed_ = other.is_closed_;
      ssl_ = other.ssl_;
      other.is_closed_ = true;
    }
    return *this;
  }
  ~TlsSocket() {
    if (!is_closed_) {
      auto _ = close();
    }
  }

private:
  TlsSocket(RawSocket underlying, ::SSL *ssl)
      : underlying_(std::move(underlying)), ssl_(ssl), is_closed_(false) {};
  RawSocket underlying_;
  ::SSL *ssl_;
  bool is_closed_;
}; // class TlsSocket
} // namespace net_utils

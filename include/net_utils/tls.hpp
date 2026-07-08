#pragma once

#include "net_utils/errors.hpp"
#include "net_utils/socket.hpp"
#include "openssl/ssl.h"
#include <functional>
namespace net_utils {
class OpenSSLCtx {
public:
  static net_utils::NetResult<std::reference_wrapper<OpenSSLCtx>> instance();
  inline ::SSL_CTX *ptr() { return ctx_; };

  OpenSSLCtx(const OpenSSLCtx &) = delete;
  OpenSSLCtx &operator=(const OpenSSLCtx &) = delete;
  OpenSSLCtx(OpenSSLCtx &&) = delete;
  OpenSSLCtx &operator=(OpenSSLCtx &&) = delete;

private:
  OpenSSLCtx();
  ~OpenSSLCtx();
  bool is_ok_ = false;
  net_utils::NetError<void> init_error_{
      net_utils::NetErrorCode::InternalSSLError};
  ::SSL_CTX *ctx_ = nullptr;
}; // class OpenSSLCtx

class TlsSocket {
public:
  static NetResult<TlsSocket, int> create(const std::string &hostname) noexcept;
  NetResult<void, int> set_flags(int flags) noexcept {
    return underlying_.set_flags(flags);
  };
  NetResult<int, int> get_flags() noexcept { return underlying_.get_flags(); };
  NetResult<void, int> connect(const SocketAddr &addr) noexcept;
  NetResult<void, int> bind(const SocketAddr &addr) noexcept {
    return underlying_.bind(addr);
  };
  NetResult<long, int> read(char *buf, std::size_t len) noexcept;
  NetResult<long, int> write(char *buf, std::size_t len) noexcept;
  NetResult<void, int> close() noexcept;
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

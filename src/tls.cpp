#include "net_utils/socket.hpp"
#include <cstdint>
#include <expected>
#include <net_utils/errors.hpp>
#include <net_utils/tls.hpp>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <sys/socket.h>

using namespace net_utils;

OpenSSLCtx::OpenSSLCtx() {

  ctx_ = ::SSL_CTX_new(::TLS_client_method());
  if (ctx_ == nullptr) {
    // error is already InternalSSLError
    // TODO: extract and return the actual error code retrieved from OpenSSL
    return;
  }

  ::SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER, nullptr);

  if (!::SSL_CTX_set_default_verify_paths(ctx_)) {
    return;
  }

  if (!::SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION)) {
    return;
  }

  is_ok_ = true;
};

OpenSSLCtx::~OpenSSLCtx() {
  if (ctx_)
    ::SSL_CTX_free(ctx_);
};

NetResult<std::reference_wrapper<OpenSSLCtx>> OpenSSLCtx::instance() {
  static OpenSSLCtx ctx;

  if (!ctx.is_ok_) {
    return std::unexpected(ctx.init_error_);
  }

  return std::ref(ctx);
};

NetResult<TlsSocket, int>
TlsSocket::create(const std::string &hostname) noexcept {
  auto ctx_opt = OpenSSLCtx::instance();
  if (!ctx_opt.has_value()) {
    return std::unexpected(NetError<int>(ctx_opt.error().code()));
  }

  OpenSSLCtx &ctx = ctx_opt.value();
  ::SSL *ssl = ::SSL_new(ctx.ptr());

  auto underlying_opt = RawSocket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!underlying_opt.has_value())
    return std::unexpected(underlying_opt.error());

  RawSocket underlying = std::move(underlying_opt.value());

  ::SSL_set_fd(ssl, underlying.fd());
  ::SSL_set_tlsext_host_name(ssl, hostname.c_str());
  ::SSL_set1_host(ssl, hostname.c_str());

  return TlsSocket(std::move(underlying), ssl);
};

NetResult<void, int> TlsSocket::connect(const SocketAddr &addr) noexcept {
  auto raw_conn = underlying_.connect(addr);
  if (!raw_conn.has_value())
    return std::unexpected(raw_conn.error());
  int rc = ::SSL_connect(ssl_);
  return {};
};

NetResult<long, int> TlsSocket::read(char *buf, std::size_t len) noexcept {
  std::size_t read;
  int rc = ::SSL_read_ex(ssl_, buf, len, &read);
  return read;
}
NetResult<long, int> TlsSocket::write(char *buf, std::size_t len) noexcept {
  std::size_t written;
  int rc = ::SSL_write_ex(ssl_, buf, len, &written);
  return written;
}

NetResult<void, int> TlsSocket::close() noexcept {
  if (is_closed_)
    return std::unexpected(NetError<int>(NetErrorCode::ConnectionClosed));
  ::SSL_shutdown(ssl_);
  ::SSL_free(ssl_);
  auto ret = underlying_.close();
  is_closed_ = true;
  return ret;
}

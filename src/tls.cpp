#include "net_utils/socket.hpp"
#include <cerrno>
#include <cstdint>
#include <expected>
#include <net_utils/errors.hpp>
#include <net_utils/tls.hpp>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <sys/socket.h>

using namespace net_utils;

OpenSSLCtx::OpenSSLCtx() {

  ctx_ = ::SSL_CTX_new(::TLS_client_method());
  if (ctx_ == nullptr) {
    std::uint64_t e = ERR_get_error();
    if (e != 0)
      init_error_ = NetError<std::uint64_t>(NetErrorCode::InternalSSLError, e);
    return;
  }

  ::SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER, nullptr);

  if (!::SSL_CTX_set_default_verify_paths(ctx_)) {
    std::uint64_t e = ERR_get_error();
    if (e != 0)
      init_error_ = NetError<std::uint64_t>(NetErrorCode::InternalSSLError, e);
    return;
  }

  if (!::SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION)) {
    std::uint64_t e = ERR_get_error();
    if (e != 0)
      init_error_ = NetError<std::uint64_t>(NetErrorCode::InternalSSLError, e);
    return;
  }

  is_ok_ = true;
};

OpenSSLCtx::~OpenSSLCtx() {
  if (ctx_)
    ::SSL_CTX_free(ctx_);
};

NetResult<std::reference_wrapper<OpenSSLCtx>, std::uint64_t>
OpenSSLCtx::instance() {
  static OpenSSLCtx ctx;

  if (!ctx.is_ok_) {
    return std::unexpected(ctx.init_error_);
  }

  return std::ref(ctx);
};

NetError<std::uint64_t> extract_SSL_error(const SSL *ssl, int ret) {
  int ssl_error_code = SSL_get_error(ssl, ret);
  switch (ssl_error_code) {
  case SSL_ERROR_SYSCALL:
    return NetError(NetErrorCode::InternalOsError,
                    static_cast<std::uint64_t>(errno));
  case SSL_ERROR_WANT_READ:
  case SSL_ERROR_WANT_WRITE:
    // TODO: Create a new error type that tells the caller to retry the
    // operation.
    return NetError(NetErrorCode::InternalSSLError, 0UL);
  case SSL_ERROR_NONE:
    return NetError(NetErrorCode::InternalSSLError, 0UL);
  case SSL_ERROR_ZERO_RETURN:
    return NetError(NetErrorCode::ConnectionClosed, 0UL);
  default:

    return NetError(NetErrorCode::InternalSSLError, ERR_get_error());
  }
}

NetResult<TlsSocket, std::uint64_t>
TlsSocket::create(const std::string &hostname) noexcept {
  auto ctx_opt = OpenSSLCtx::instance();
  if (!ctx_opt.has_value()) {
    return std::unexpected(NetError<std::uint64_t>(ctx_opt.error().code()));
  }

  OpenSSLCtx &ctx = ctx_opt.value();
  ::SSL *ssl = ::SSL_new(ctx.ptr());
  if (ssl == nullptr)
    return std::unexpected(
        NetError(NetErrorCode::InternalSSLError, ERR_get_error()));

  auto underlying_opt = RawSocket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!underlying_opt.has_value())
    return std::unexpected(underlying_opt.error());

  RawSocket underlying = std::move(underlying_opt.value());

  int rc = ::SSL_set_fd(ssl, underlying.fd());
  if (rc != 1) {
    SSL_free(ssl);
    return std::unexpected(extract_SSL_error(ssl, rc));
  }
  rc = ::SSL_set_tlsext_host_name(ssl, hostname.c_str());
  if (rc != 1) {
    SSL_free(ssl);
    return std::unexpected(extract_SSL_error(ssl, rc));
  }
  rc = ::SSL_set1_host(ssl, hostname.c_str());
  if (rc != 1) {
    SSL_free(ssl);
    return std::unexpected(extract_SSL_error(ssl, rc));
  }

  return TlsSocket(std::move(underlying), ssl);
};

NetResult<void, std::uint64_t>
TlsSocket::connect(const SocketAddr &addr) noexcept {
  auto raw_conn = underlying_.connect(addr);
  if (!raw_conn.has_value())
    return std::unexpected(raw_conn.error());
  int rc = ::SSL_connect(ssl_);
  if (rc != 1)
    return std::unexpected(extract_SSL_error(ssl_, rc));
  return {};
};

NetResult<std::size_t, std::uint64_t>
TlsSocket::read(char *buf, std::size_t len) noexcept {
  std::size_t read;
  int rc = ::SSL_read_ex(ssl_, buf, len, &read);
  if (rc != 1)
    return std::unexpected(extract_SSL_error(ssl_, rc));
  return read;
}
NetResult<std::size_t, std::uint64_t>
TlsSocket::write(const char *buf, std::size_t len) noexcept {
  std::size_t written;
  int rc = ::SSL_write_ex(ssl_, buf, len, &written);
  if (rc != 1)
    return std::unexpected(extract_SSL_error(ssl_, rc));
  return written;
}

NetResult<void, std::uint64_t> TlsSocket::close() noexcept {
  if (is_closed_)
    return std::unexpected(
        NetError<std::uint64_t>(NetErrorCode::ConnectionClosed));
  // ignore the result of shutdown since it can return zero if
  // shutdown was sent but not received back
  auto _ = ::SSL_shutdown(ssl_);
  ::SSL_free(ssl_);
  auto ret = underlying_.close();
  is_closed_ = true;
  return ret;
}

std::string TlsSocket::error_decode_helper(std::uint64_t error_code) noexcept {
  char buf[256];
  ERR_error_string_n(error_code, buf, sizeof(buf));
  return std::string(buf);
}

#pragma once

#include "net_utils/errors.hpp"
#include "openssl/ssl.h"
#include <functional>
#include <openssl/prov_ssl.h>

class OpenSSLCtx {
  static net_utils::NetResult<std::reference_wrapper<OpenSSLCtx>> instance() {
    static OpenSSLCtx ctx;

    if (!ctx.is_ok_) {
      return std::unexpected(ctx.init_error_);
    }

    return std::ref(ctx);
  };

  OpenSSLCtx(const OpenSSLCtx &) = delete;
  OpenSSLCtx &operator=(const OpenSSLCtx &) = delete;
  OpenSSLCtx(OpenSSLCtx &&) = delete;
  OpenSSLCtx &operator=(OpenSSLCtx &&) = delete;

private:
  OpenSSLCtx() {
    ctx_ = SSL_CTX_new(TLS_client_method());
    if (ctx_ == nullptr) {
      // error is already InternalSSLError
      // TODO: extract and return the actual error code retrieved from OpenSSL
      return;
    }

    SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER, nullptr);

    if (!SSL_CTX_set_default_verify_paths(ctx_)) {
      return;
    }

    if (!SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION)) {
      return;
    }

    is_ok_ = true;
  };

  ~OpenSSLCtx() {
    if (ctx_)
      SSL_CTX_free(ctx_);
  }

  bool is_ok_ = false;
  net_utils::NetError<void> init_error_{
      net_utils::NetErrorCode::InternalSSLError};
  SSL_CTX *ctx_ = nullptr;
}; // class OpenSSLCtx

class TlsSocket {}; // class TlsSocket

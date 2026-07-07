#pragma once

#include "net_utils/errors.hpp"
#include "openssl/ssl.h"
#include <functional>

class OpenSSLCtx {
  net_utils::NetResult<std::reference_wrapper<OpenSSLCtx>> instance();

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
  SSL_CTX *ctx_ = nullptr;
}; // class OpenSSLCtx

class TlsSocket {}; // class TlsSocket

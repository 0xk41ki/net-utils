#include <net_utils/errors.hpp>
#include <net_utils/tls.hpp>

using namespace net_utils;

OpenSSLCtx::OpenSSLCtx() {

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

OpenSSLCtx::~OpenSSLCtx() {
  if (ctx_)
    SSL_CTX_free(ctx_);
};

NetResult<std::reference_wrapper<OpenSSLCtx>> OpenSSLCtx::instance() {
  static OpenSSLCtx ctx;

  if (!ctx.is_ok_) {
    return std::unexpected(ctx.init_error_);
  }

  return std::ref(ctx);
};

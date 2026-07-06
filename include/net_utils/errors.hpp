#pragma once
#include <expected>
#include <string_view>

namespace net_utils {
enum class NetErrorCode {
  AddressResolutionFailed,
  AllocFailure,
  InternalOsError,
  ConnectionClosed,
  ConnectionNotEstablished,
  InternalSSLError,
};

constexpr std::string_view to_string(NetErrorCode code) {
  switch (code) {
  case NetErrorCode::AddressResolutionFailed:
    return "AddressResolutionFailed";
  case NetErrorCode::AllocFailure:
    return "AllocFailure";
  case NetErrorCode::InternalOsError:
    return "InternalOsError";
  case NetErrorCode::ConnectionClosed:
    return "ConnectionClosed";
  case NetErrorCode::ConnectionNotEstablished:
    return "ConnectionNotEstablished";
  case NetErrorCode::InternalSSLError:
    return "InternalSSLError";
  }

  return "UnknownError";
}

template <typename T> class NetError {
public:
  explicit NetError(NetErrorCode code) : code_(code) {};
  NetError(NetErrorCode code, T data) : code_(code), data_(data) {};
  NetErrorCode code() const noexcept { return code_; };
  const T &data() const noexcept { return data_; };

private:
  NetErrorCode code_;
  T data_;
};

template <> class NetError<void> {
public:
  explicit NetError(NetErrorCode code) : code_(code) {};
  NetErrorCode code() const noexcept { return code_; };

private:
  NetErrorCode code_;
};

template <typename T, typename E = void>
using NetResult = std::expected<T, NetError<E>>;
}; // namespace net_utils

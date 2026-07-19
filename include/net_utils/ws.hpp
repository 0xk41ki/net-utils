#pragma once
#include "net_utils/errors.hpp"
#include "net_utils/tls.hpp"
#include <cstddef>
#include <cstdint>
#include <new>
#include <string>
#include <utility>
#include <vector>
namespace net_utils {
namespace websockets {

static constexpr std::size_t DEFAULT_READ_BUFFER_LENGTH = 4096;

enum class FrameType { Binary, Text, Ping, Pong, Close, Unknown };

class WsMessageBorrowed {
public:
  std::span<std::byte> payload() const noexcept {
    return {buffer_.data() + data_offset_, data_length_};
  };
  FrameType frame_type() const noexcept {
    switch (buffer_.data()[0] & std::byte{0x0f}) {
    case std::byte{0x01}:
      return FrameType::Text;
    case std::byte{0x02}:
      return FrameType::Binary;
    case std::byte{0x08}:
      return FrameType::Close;
    case std::byte{0x09}:
      return FrameType::Ping;
    case std::byte{0x0A}:
      return FrameType::Pong;
    default:
      return FrameType::Unknown;
    }
  }
  bool fin() const noexcept {
    return (buffer_.data()[0] & std::byte{0b10000000}) != std::byte{0};
  }

  bool rsv1() const noexcept {
    return (buffer_.data()[0] & std::byte{0b01000000}) != std::byte{0};
  }
  bool rsv2() const noexcept {
    return (buffer_.data()[0] & std::byte{0b00100000}) != std::byte{0};
  }
  bool rsv3() const noexcept {
    return (buffer_.data()[0] & std::byte{0b00010000}) != std::byte{0};
  }
  bool masked() const noexcept {
    return (buffer_.data()[1] & std::byte{0b10000000}) != std::byte{0};
  }
  WsMessageBorrowed(std::span<std::byte> buffer, std::size_t data_offset,
                    std::size_t data_length)
      : buffer_(buffer), data_offset_(data_offset),
        data_length_(data_length) {};

private:
  std::span<std::byte> buffer_;
  std::size_t data_offset_;
  std::size_t data_length_;
};

class WsConfig {
public:
  WsConfig(std::string hostname, std::string port, std::string path)
      : hostname_(hostname), port_(port), path_(path) {};
  void set_hostname(std::string hostname) noexcept {
    hostname_ = std::move(hostname);
  };
  void set_port(std::string port) noexcept { port_ = std::move(port); };
  void set_path(std::string path) noexcept { path_ = std::move(path); };
  void add_header(std::string name, std::string value) {
    headers_.push_back({std::move(name), std::move(value)});
  };
  void set_read_buffer_length(std::size_t length) noexcept {
    read_buffer_length_ = length;
  };

  std::string hostname() const noexcept { return hostname_; };
  std::string port() const noexcept { return port_; };
  std::string path() const noexcept { return path_; };
  std::vector<std::pair<std::string, std::string>> headers() const noexcept {
    return headers_;
  };
  std::size_t read_buffer_length() const noexcept {
    return read_buffer_length_;
  };

private:
  std::string hostname_;
  std::string port_;
  std::string path_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::size_t read_buffer_length_ = DEFAULT_READ_BUFFER_LENGTH;
};

class SecureWs {
public:
  static NetResult<SecureWs, std::uint64_t> create(WsConfig config) noexcept;
  NetResult<void, std::uint64_t> connect() noexcept;
  NetResult<WsMessageBorrowed, std::uint64_t> read() noexcept;
  NetResult<void, std::uint64_t> write() noexcept;
  NetResult<void, std::uint64_t> close() noexcept;

private:
  SecureWs(TlsSocket socket, WsConfig config)
      : socket_(std::move(socket)), config_(std::move(config)),
        read_buffer_(new (std::nothrow) std::byte[config.read_buffer_length()]),
        read_cursor_(0), read_length_(0) {};

  TlsSocket socket_;
  WsConfig config_;
  std::byte *read_buffer_;
  std::size_t read_cursor_;
  std::size_t read_length_;
};

} // namespace websockets
} // namespace net_utils

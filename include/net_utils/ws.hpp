#pragma once
#include "net_utils/errors.hpp"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
namespace net_utils {
namespace websockets {

enum class FrameType { Binary, Text, Ping, Pong, Close };

class WsConnectConfig {
public:
  WsConnectConfig(std::string hostname, std::string port, std::string path)
      : hostname_(hostname), port_(port), path_(path) {};
  void set_hostname(std::string hostname) noexcept {
    hostname_ = std::move(hostname);
  };
  void set_port(std::string port) noexcept { port_ = std::move(port); };
  void set_path(std::string path) noexcept { path_ = std::move(path); };
  void add_header(std::string name, std::string value) {
    headers_.push_back({std::move(name), std::move(value)});
  };

private:
  std::string hostname_;
  std::string port_;
  std::string path_;
  std::vector<std::pair<std::string, std::string>> headers_;
};

class SecureWs {
public:
  static NetResult<SecureWs, std::uint64_t>
  create(const WsConnectConfig &config) noexcept;
  NetResult<std::uint64_t> connect() noexcept;
  NetResult<std::uint64_t> read() noexcept;
  NetResult<std::uint64_t> write() noexcept;
};

} // namespace websockets
} // namespace net_utils

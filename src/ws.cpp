#include "net_utils/ws.hpp"
#include "net_utils/dns.hpp"
#include "net_utils/errors.hpp"
#include "net_utils/tls.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <optional>
#include <sys/socket.h>

using namespace net_utils;
using namespace net_utils::websockets;

NetResult<SecureWs, std::uint64_t> SecureWs::create(WsConfig config) noexcept {
  auto socket_opt = TlsSocket::create(config.hostname());
  if (!socket_opt.has_value())
    return std::unexpected(socket_opt.error());

  return SecureWs(std::move(socket_opt.value()), std::move(config));
};

NetResult<void, std::uint64_t> SecureWs::connect() noexcept {
  DnsResolver resolver(config_.hostname(), config_.port());
  auto resolver_res = resolver.resolve();
  if (!resolver_res.has_value())
    return std::unexpected(resolver_res.error());
  std::size_t num_addresses = resolver_res.value();
  std::size_t chosen_address = SIZE_MAX;
  for (std::size_t i = 0; i < num_addresses; i++)
    if (resolver.get_result_at(i).family == AF_INET) {
      chosen_address = i;
      break;
    }
  if (chosen_address >= num_addresses)
    return std::unexpected(
        NetError(NetErrorCode::AddressResolutionFailed, 0UL));

  auto socket_connect_result =
      socket_.connect(resolver.get_result_at(chosen_address).address);
  if (!socket_connect_result.has_value())
    return std::unexpected(socket_connect_result.error());

  std::string upgrade_request =
      "GET " + config_.path() + " HTTP/1.1\r\n" +
      "Host: " + config_.hostname() + "\r\n" + "Connection: Upgrade\r\n" +
      "Upgrade: websocket\r\n" +
      "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
  auto write_res = socket_.write(
      {reinterpret_cast<const std::byte *>(upgrade_request.data()),
       upgrade_request.length()});
  if (!write_res.has_value())
    return std::unexpected(write_res.error());
  if (write_res.value() != upgrade_request.length())
    return std::unexpected(NetError(NetErrorCode::InternalOsError,
                                    0UL)); // TODO: Handle this correctly

  while (true) {

    auto read_res = socket_.read({read_buffer_ + read_length_,
                                  config_.read_buffer_length() - read_length_});
    if (!read_res.has_value())
      return std::unexpected(read_res.error());
    read_length_ += read_res.value();
    // TODO: actually parse the http response rather than just checking when the
    // request has ended
    if (read_length_ >= 4 &&
        std::memcmp(read_buffer_ + read_length_ - 4, "\r\n\r\n", 4) == 0) {
      read_cursor_ = read_length_;
      break;
    }
  }
  return {};
};

std::optional<WsMessageBorrowed> try_parse_frame(std::byte *buffer,
                                                 std::size_t read_length,
                                                 std::size_t &read_cursor) {
  auto length = buffer[read_cursor + 1];
  std::size_t header_length = 2;
  std::uint64_t data_length = 0;
  std::byte *data_length_ptr = reinterpret_cast<std::byte *>(&data_length);
  if (length <= std::byte{125})
    data_length_ptr[0] = length;
  else if (length == std::byte{126}) {
    data_length_ptr[0] = buffer[read_cursor + 3];
    data_length_ptr[1] = buffer[read_cursor + 2];
    header_length += 2;
  } else {
    data_length_ptr[0] = buffer[read_cursor + 9];
    data_length_ptr[1] = buffer[read_cursor + 8];
    data_length_ptr[2] = buffer[read_cursor + 7];
    data_length_ptr[3] = buffer[read_cursor + 6];
    data_length_ptr[4] = buffer[read_cursor + 5];
    data_length_ptr[5] = buffer[read_cursor + 4];
    data_length_ptr[6] = buffer[read_cursor + 3];
    data_length_ptr[7] = buffer[read_cursor + 2];
    header_length += 8;
  }

  if (read_cursor + header_length + data_length > read_length) {
    // haven't received the full frame yet
    return std::nullopt;
  };

  std::size_t cursor_begin = read_cursor;
  read_cursor += header_length + data_length;
  return WsMessageBorrowed({buffer + cursor_begin, header_length + data_length},
                           header_length, data_length);
};

NetResult<WsMessageBorrowed, std::uint64_t> SecureWs::read() noexcept {
  if (read_length_ == read_cursor_) {
    read_length_ = read_cursor_ = 0;
  }

  // read until we have a frame
  while (true) {
    auto frame_opt = try_parse_frame(read_buffer_, read_length_, read_cursor_);
    if (frame_opt.has_value())
      return frame_opt.value();
    // this means that the buffer is full
    if (read_length_ == config_.read_buffer_length()) {
      // The buffer is too small
      if (read_cursor_ == 0)
        return std::unexpected(NetError(NetErrorCode::BufferTooSmall, 0ul));
      // try to free the part of the buffer that has already been processed
      std::size_t offset = read_cursor_;
      for (std::size_t i = read_cursor_; i < read_length_; i++) {
        read_buffer_[i - offset] = read_buffer_[i];
      }
      read_length_ = read_length_ - offset;
      read_cursor_ = 0;
    }
    auto read_res = socket_.read({read_buffer_ + read_length_,
                                  config_.read_buffer_length() - read_length_});
    if (!read_res.has_value())
      return std::unexpected(read_res.error());
    read_length_ += read_res.value();
  }
}

NetResult<void, std::uint64_t> SecureWs::write() noexcept { return {}; };
NetResult<void, std::uint64_t> SecureWs::close() noexcept { return {}; };

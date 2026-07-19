#include "net_utils/errors.hpp"
#include "net_utils/ws.hpp"
#include <ios>
#include <iostream>

int main() {
  net_utils::websockets::WsConfig config(
      "stream.binance.com", "443",
      "/stream?streams=btcusdt@trade/btcusdt@depth20@100ms");
  auto ws_res = net_utils::websockets::SecureWs::create(std::move(config));
  if (!ws_res) {
    std::cout << "error in init: "
              << net_utils::to_string(ws_res.error().code())
              << " data: " << ws_res.error().data() << std::endl;
    return -1;
  }
  auto ws = std::move(ws_res.value());
  auto connect_res = ws.connect();
  if (!connect_res) {
    std::cout << "error in connect: "
              << net_utils::to_string(connect_res.error().code())
              << " data: " << connect_res.error().data() << std::endl;
    return -1;
  }

  while (true) {
    auto maybe_frame = ws.read();
    if (!maybe_frame) {

      std::cout << "error in read: "
                << net_utils::to_string(maybe_frame.error().code())
                << " data: " << maybe_frame.error().data() << std::endl;
      return -1;
    }
    auto frame = std::move(maybe_frame.value());
    switch (frame.frame_type()) {
    case net_utils::websockets::FrameType::Text:
      std::cout << "[] read text frame: " << frame.payload().data() << " \n";
      std::cout.write(reinterpret_cast<const char *>(frame.payload().data()),
                      static_cast<std::streamsize>(frame.payload().size()));
      std::cout << std::endl;
      break;
    default:
      std::cout << "received frame of other type" << std::endl;
    }
  }
  return 0;
}

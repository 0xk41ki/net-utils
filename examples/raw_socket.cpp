#include <cstddef>
#include <iostream>
#include <net_utils/dns.hpp>
#include <net_utils/socket.hpp>
#include <string>
#include <sys/socket.h>
#include <utility>

using namespace net_utils;

int main() {
  const std::string host = "example.com";
  const std::string port = "80";

  DnsResolver resolver(host, port);
  auto res = resolver.resolve();
  if (!res.has_value()) {
    std::cout << "dns resolution failed: " << to_string(res.error().code())
              << std::endl;
    return -1;
  }

  const AddrInfo *chosen = nullptr;
  for (std::size_t i = 0; i < res.value(); i++) {
    const AddrInfo &ai = resolver.get_result_at(i);
    if (ai.family == AF_INET) {
      chosen = &ai;
      break;
    }
  }
  if (chosen == nullptr) {
    std::cout << "no IPv4 address found for " << host << std::endl;
    return -1;
  }
  std::cout << "connecting to " << chosen->address.to_string() << ":" << port
            << std::endl;

  auto sres =
      RawSocket::create(chosen->family, chosen->socktype, chosen->protocol);
  if (!sres.has_value()) {
    std::cout << "socket creation failed (errno=" << sres.error().data() << ")"
              << std::endl;
    return -1;
  }
  RawSocket sock = std::move(*sres);

  if (auto c = sock.connect(chosen->address); !c.has_value()) {
    std::cout << "connect failed (errno=" << c.error().data() << ")"
              << std::endl;
    return -1;
  }

  std::string request =
      "GET / HTTP/1.0\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
  if (auto w = sock.write(request.data(), request.size()); !w.has_value()) {
    std::cout << "write failed: " << to_string(w.error().code()) << std::endl;
    return -1;
  }

  std::cout << "--- response ---" << std::endl;
  char buf[4096];
  while (true) {
    auto r = sock.read(buf, sizeof(buf));
    if (!r.has_value()) {
      if (r.error().code() == NetErrorCode::ConnectionClosed) {
        break; // clean EOF
      }
      std::cout << "\nread failed: " << to_string(r.error().code())
                << std::endl;
      return -1;
    }
    std::cout.write(buf, *r);
  }
  std::cout << "\n--- end ---" << std::endl;

  return 0;
}

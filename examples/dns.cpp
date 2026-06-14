#include "net_utils/errors.hpp"
#include <cstddef>
#include <iostream>
#include <net_utils/dns.hpp>

int main() {
  net_utils::DnsResolver resolver("amazonaws.com", "443");
  auto res = resolver.resolve();
  if (!res.has_value()) {
    std::cout << "failed to resolved dns due to error: "
              << to_string(res.error().code()) << std::endl;
    return -1;
  }
  std::size_t num = resolver.num_results();
  std::cout << "addr resolved to " << num << " addresses" << std::endl;
  for (std::size_t i = 0; i < num; i++) {
    std::cout << i << " = " << resolver.get_result_at(i).to_string()
              << std::endl;
  }
  return 0;
}

#include <cstddef>
#include <iostream>
#include <net_utils/dns.hpp>

int main() {
  net_utils::DnsResolver resolver("artha.vaquita-grue.ts.net", "443");
  resolver.resolve();
  std::size_t num = resolver.num_results();
  std::cout << "addr resolved to " << num << " addresses" << std::endl;
  for (std::size_t i = 0; i < num; i++) {
    std::cout << i << " = " << resolver.get_result_at(i).to_string()
              << std::endl;
  }
  return 0;
}

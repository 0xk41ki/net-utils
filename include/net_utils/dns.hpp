#include <arpa/inet.h>
#include <cstddef>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <vector>
namespace net_utils {
struct DnsResolutionResult {
  int family_;
  int socktype_;
  int protocol_;
  socklen_t addr_len_;
  struct sockaddr *addr_;

public:
  DnsResolutionResult(int family, int socktype, int protocol,
                      socklen_t addr_len, struct sockaddr *addr);
  ~DnsResolutionResult();
  DnsResolutionResult(const DnsResolutionResult &);
  DnsResolutionResult &operator=(const DnsResolutionResult &);
  DnsResolutionResult(DnsResolutionResult &&);
  DnsResolutionResult &operator=(DnsResolutionResult &&);
  inline std::string to_string() const {
    char *buf = new char[INET6_ADDRSTRLEN];
    inet_ntop(family_, &addr_, buf, INET6_ADDRSTRLEN);
    return std::string(buf, buf + INET6_ADDRSTRLEN);
  }
};

class DnsResolver {
public:
  DnsResolver(std::string host, std::string port)
      : host_(std::move(host)), port_(std::move(port)) {};
  void resolve();
  std::size_t num_results() const noexcept;
  const DnsResolutionResult &get_result_at(std::size_t) const noexcept;

private:
  std::string host_;
  std::string port_;
  std::vector<DnsResolutionResult> addrs_;
}; // class DnsResolver

} // namespace net_utils

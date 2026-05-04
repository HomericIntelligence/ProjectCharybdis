#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace projectcharybdis {

/// Thin HTTP client for chaos/resilience GTest tests.
/// Wraps cpp-httplib for REST API interactions with Agamemnon.
class HttpTestClient {
 public:
  static constexpr std::size_t kMaxBodyBytes = 10 * 1024 * 1024;  // 10 MB

  explicit HttpTestClient(const std::string& base_url = "http://localhost:8080");

  /// GET request, returns {status_code, body_json}
  struct Response {
    int status;
    nlohmann::json body;
  };

  Response get(const std::string& path) const;
  Response post(const std::string& path, const nlohmann::json& body = {}) const;
  Response del(const std::string& path) const;

  /// POST with raw string body (for malformed payload tests)
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  Response post_raw(const std::string& path, const std::string& body,
                    const std::string& content_type = "application/json") const;

  bool is_healthy() const;

 private:
  std::string host_;
  int port_;
};

}  // namespace projectcharybdis

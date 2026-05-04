#include "projectcharybdis/http_test_client.hpp"

#include <httplib.h>
#include <regex>
#include <stdexcept>
#include <string>

namespace projectcharybdis {

HttpTestClient::HttpTestClient(const std::string& base_url) : host_("localhost"), port_(8080) {
  std::regex url_re(R"(https?://([^:]+):(\d+))");
  std::smatch match{};  // NOLINT(misc-const-correctness)
  if (std::regex_match(base_url, match, url_re)) {
    host_ = match[1].str();
    try {
      port_ = std::stoi(match[2].str());
    } catch (const std::invalid_argument& e) {
      throw std::runtime_error("HttpTestClient: invalid port '" + match[2].str() +
                               "': " + e.what());
    } catch (const std::out_of_range& e) {
      throw std::runtime_error("HttpTestClient: port out of range '" + match[2].str() +
                               "': " + e.what());
    }
    if (port_ < 1 || port_ > 65535) {
      throw std::runtime_error("HttpTestClient: port out of valid range [1,65535]: " +
                               match[2].str());
    }
  } else {
    host_ = "localhost";
    port_ = 8080;
  }
}

HttpTestClient::Response HttpTestClient::get(const std::string& path) const {
  httplib::Client cli(host_, port_);
  cli.set_connection_timeout(5);
  cli.set_read_timeout(10);

  auto res = cli.Get(path);
  if (!res) { return {0, {}}; }
  if (res->body.size() > kMaxBodyBytes) { return {res->status, {{"error", "response_too_large"}}}; }

  nlohmann::json body;
  try {
    body = nlohmann::json::parse(res->body);
  } catch (...) {
    body = {{"raw", res->body}};
  }
  return {res->status, body};
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
HttpTestClient::Response HttpTestClient::post(const std::string& path, const nlohmann::json& body) const {
  return post_raw(path, body.dump(), "application/json");
}

HttpTestClient::Response HttpTestClient::del(const std::string& path) const {
  httplib::Client cli(host_, port_);
  cli.set_connection_timeout(5);
  cli.set_read_timeout(10);

  auto res = cli.Delete(path);
  if (!res) { return {0, {}}; }
  if (res->body.size() > kMaxBodyBytes) { return {res->status, {{"error", "response_too_large"}}}; }

  nlohmann::json resp_body;
  try {
    resp_body = nlohmann::json::parse(res->body);
  } catch (...) {
    resp_body = {{"raw", res->body}};
  }
  return {res->status, resp_body};
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
HttpTestClient::Response HttpTestClient::post_raw(const std::string& path, const std::string& body,
                                                  const std::string& content_type) const {
  httplib::Client cli(host_, port_);
  cli.set_connection_timeout(5);
  cli.set_read_timeout(10);

  auto res = cli.Post(path, body, content_type);
  if (!res) { return {0, {}}; }
  if (res->body.size() > kMaxBodyBytes) { return {res->status, {{"error", "response_too_large"}}}; }

  nlohmann::json resp_body;
  try {
    resp_body = nlohmann::json::parse(res->body);
  } catch (...) {
    resp_body = {{"raw", res->body}};
  }
  return {res->status, resp_body};
}

bool HttpTestClient::is_healthy() const {
  auto [status, body] = get("/v1/health");
  return status == 200 && body.value("status", "") == "ok";
}

}  // namespace projectcharybdis

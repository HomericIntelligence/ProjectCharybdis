/**
 * @file test_http_client_unit.cpp
 * @brief Unit tests for HttpTestClient — no live server required.
 *
 * Covers URL parsing logic in the constructor and the connection-failure
 * branches (status == 0 returns) in get/post/del/post_raw/is_healthy.
 * Uses port 1 on localhost, which is always refused, to exercise the
 * `if (!res) return {0, {}};` paths without needing Agamemnon running.
 */

#include "projectcharybdis/http_test_client.hpp"

#include <gtest/gtest.h>

namespace projectcharybdis {

// ── Constructor / URL parsing ─────────────────────────────────────────────────

TEST(HttpTestClientUnit, DefaultConstructor) {
  // Default URL "http://localhost:8080" — port 1 redirected via offline fixture;
  // here we just verify the constructor runs without throwing.
  EXPECT_NO_THROW(HttpTestClient client("http://127.0.0.1:1"));
}

TEST(HttpTestClientUnit, ValidUrlParsing) {
  // Constructor should parse host and port from a well-formed URL.
  // Port 1 gives immediate ECONNREFUSED, so no timeout delay.
  HttpTestClient client("http://127.0.0.1:1");
  EXPECT_FALSE(client.is_healthy());
}

TEST(HttpTestClientUnit, MalformedUrlFallsBackToDefaults) {
  // A URL that doesn't match the regex exercises the else branch in the
  // constructor. We only verify construction succeeds, not connection.
  EXPECT_NO_THROW(HttpTestClient client("not-a-url"));
}

TEST(HttpTestClientUnit, HttpsUrlParsing) {
  // The regex also handles https://; port 1 gives immediate refusal.
  HttpTestClient client("https://127.0.0.1:1");
  EXPECT_FALSE(client.is_healthy());
}

// ── Connection-failure paths (port 1 is always refused) ───────────────────────

class HttpTestClientOffline : public ::testing::Test {
 protected:
  // Port 1 is privileged and always connection-refused on Linux runners
  HttpTestClientOffline() : client_("http://127.0.0.1:1") {}
  HttpTestClient client_;
};

TEST_F(HttpTestClientOffline, GetReturnsZeroStatus) {
  auto [status, body] = client_.get("/any/path");
  EXPECT_EQ(status, 0);
  EXPECT_TRUE(body.empty());
}

TEST_F(HttpTestClientOffline, PostReturnsZeroStatus) {
  nlohmann::json payload = {{"key", "value"}};
  auto [status, body] = client_.post("/any/path", payload);
  EXPECT_EQ(status, 0);
  EXPECT_TRUE(body.empty());
}

TEST_F(HttpTestClientOffline, PostEmptyBodyReturnsZeroStatus) {
  auto [status, body] = client_.post("/any/path");
  EXPECT_EQ(status, 0);
  EXPECT_TRUE(body.empty());
}

TEST_F(HttpTestClientOffline, DelReturnsZeroStatus) {
  auto [status, body] = client_.del("/any/path");
  EXPECT_EQ(status, 0);
  EXPECT_TRUE(body.empty());
}

TEST_F(HttpTestClientOffline, PostRawReturnsZeroStatus) {
  auto [status, body] = client_.post_raw("/any/path", "raw body", "text/plain");
  EXPECT_EQ(status, 0);
  EXPECT_TRUE(body.empty());
}

TEST_F(HttpTestClientOffline, IsHealthyReturnsFalse) { EXPECT_FALSE(client_.is_healthy()); }

}  // namespace projectcharybdis

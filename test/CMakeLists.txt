cmake_minimum_required(VERSION 3.20)

find_package(GTest REQUIRED)
find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)

# ── Version tests (always runnable) ──────────────────────────────────────────
add_executable(${PROJECT_NAME}_tests
    src/test_http_client_unit.cpp
    src/test_helpers_unit.cpp)

target_compile_features(${PROJECT_NAME}_tests PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME}_tests PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME}_tests
  PRIVATE
    ${PROJECT_NAME}::${PROJECT_NAME}
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)

# ── Integration tests (require Agamemnon + NATS running) ─────────────────────
add_executable(${PROJECT_NAME}_integration_tests
    src/test_chaos_api.cpp
    src/test_payload_fuzzing.cpp
    src/test_malformed_messages.cpp
    src/test_protocol_correctness.cpp
    src/test_chaos_resilience.cpp)

target_compile_features(${PROJECT_NAME}_integration_tests PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME}_integration_tests PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME}_integration_tests
  PRIVATE
    ${PROJECT_NAME}::${PROJECT_NAME}
    httplib::httplib
    nlohmann_json::nlohmann_json
    GTest::gtest_main)

# Discover all integration tests except R04 (which needs an extra label).
# gtest_discover_tests runs POST_BUILD, so set_tests_properties cannot be used
# after the call — tests don't exist at configure time. Instead, use two
# separate discovery calls with gtest_filter to partition the test suite.
gtest_discover_tests(${PROJECT_NAME}_integration_tests
    TEST_SUFFIX .nats
    EXTRA_ARGS "--gtest_filter=-ChaosResilienceTest.R04_QueueStarve_ConsumerStalls"
    PROPERTIES LABELS "REQUIRES_NATS")

# R04_QueueStarve_ConsumerStalls additionally requires a live myrmidon pull consumer.
# Without one, the task will never advance to 'completed' and the recovery assertion
# will always time out. CI environments without a myrmidon agent can skip with:
#   ctest --label-exclude REQUIRES_MYRMIDON
gtest_discover_tests(${PROJECT_NAME}_integration_tests
    TEST_SUFFIX .myrmidon
    EXTRA_ARGS "--gtest_filter=ChaosResilienceTest.R04_QueueStarve_ConsumerStalls"
    PROPERTIES LABELS "REQUIRES_NATS;REQUIRES_MYRMIDON")

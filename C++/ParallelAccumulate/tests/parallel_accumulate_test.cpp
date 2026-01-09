#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "parallel_accumulate.hpp"
#include <vector>
#include <numeric>

// =============================================================================
// Basic Functionality Tests
// =============================================================================

TEST_CASE("parallel_accumulate basic functionality", "[parallel_accumulate][basic]") {
  SECTION("empty vector returns init") {
    std::vector<int> v;
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0) == 0);
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 10) == 10);
  }

  SECTION("single element") {
    std::vector<int> v = {42};
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0) == 42);
  }

  SECTION("small vector") {
    std::vector<int> v = {1, 2, 3, 4, 5};
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0) == 15);
  }

  SECTION("matches std::accumulate") {
    std::vector<int> v(1000);
    std::iota(v.begin(), v.end(), 1);  // 1 to 1000
    int expected = std::accumulate(v.begin(), v.end(), 0);
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0) == expected);
  }

  SECTION("with non-zero init") {
    std::vector<int> v = {1, 2, 3};
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 100) == 106);
  }
}

// =============================================================================
// Padded Version Tests
// =============================================================================

TEST_CASE("parallel_accumulate_padded functionality", "[parallel_accumulate][padded]") {
  SECTION("empty vector returns init") {
    std::vector<int> v;
    REQUIRE(parallel_accumulate_padded(v.begin(), v.end(), 0) == 0);
  }

  SECTION("matches std::accumulate for large vector") {
    std::vector<int> v(10000);
    std::iota(v.begin(), v.end(), 1);
    int expected = std::accumulate(v.begin(), v.end(), 0);
    REQUIRE(parallel_accumulate_padded(v.begin(), v.end(), 0) == expected);
  }

  SECTION("with doubles") {
    std::vector<double> v = {1.5, 2.5, 3.0, 4.0};
    double expected = std::accumulate(v.begin(), v.end(), 0.0);
    REQUIRE(parallel_accumulate_padded(v.begin(), v.end(), 0.0) == expected);
  }
}

// =============================================================================
// Divide and Conquer Tests
// =============================================================================

TEST_CASE("parallel_accumulate_dc functionality", "[parallel_accumulate][dc]") {
  SECTION("small vector (below cutoff)") {
    std::vector<int> v(100);
    std::iota(v.begin(), v.end(), 1);
    int expected = std::accumulate(v.begin(), v.end(), 0);
    REQUIRE(parallel_accumulate_dc(v.begin(), v.end(), 0) == expected);
  }

  SECTION("large vector (above cutoff)") {
    std::vector<int> v(50000);
    std::iota(v.begin(), v.end(), 1);
    int expected = std::accumulate(v.begin(), v.end(), 0);
    REQUIRE(parallel_accumulate_dc(v.begin(), v.end(), 0) == expected);
  }
}

// =============================================================================
// STL Parallel Version Tests
// =============================================================================

TEST_CASE("parallel_accumulate_std functionality", "[parallel_accumulate][std]") {
  SECTION("matches std::accumulate") {
    std::vector<int> v(10000);
    std::iota(v.begin(), v.end(), 1);
    int expected = std::accumulate(v.begin(), v.end(), 0);
    REQUIRE(parallel_accumulate_std(v.begin(), v.end(), 0) == expected);
  }

  SECTION("with long long to avoid overflow") {
    std::vector<long long> v(100000);
    std::iota(v.begin(), v.end(), 1LL);
    long long expected = std::accumulate(v.begin(), v.end(), 0LL);
    REQUIRE(parallel_accumulate_std(v.begin(), v.end(), 0LL) == expected);
  }
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_CASE("parallel_accumulate edge cases", "[parallel_accumulate][edge]") {
  SECTION("negative numbers") {
    std::vector<int> v = {-1, -2, -3, 4, 5};
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0) == 3);
  }

  SECTION("all zeros") {
    std::vector<int> v(1000, 0);
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0) == 0);
  }

  SECTION("alternating positive and negative") {
    std::vector<int> v;
    for (int i = 0; i < 1000; ++i) {
      v.push_back(i % 2 == 0 ? i : -i);
    }
    int expected = std::accumulate(v.begin(), v.end(), 0);
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0) == expected);
  }
}

// =============================================================================
// Type Tests
// =============================================================================

TEST_CASE("parallel_accumulate with different types", "[parallel_accumulate][types]") {
  SECTION("float") {
    std::vector<float> v = {1.1f, 2.2f, 3.3f};
    float expected = std::accumulate(v.begin(), v.end(), 0.0f);
    float result = parallel_accumulate(v.begin(), v.end(), 0.0f);
    REQUIRE(result == Catch::Approx(expected).epsilon(0.001));
  }

  SECTION("unsigned long") {
    std::vector<unsigned long> v = {100UL, 200UL, 300UL};
    REQUIRE(parallel_accumulate(v.begin(), v.end(), 0UL) == 600UL);
  }
}

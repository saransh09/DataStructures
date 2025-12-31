#include "thread_safe_queue.hpp"
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// ------ BASIC OPERATIONS -------

TEST_CASE("push and try_pop single element", "[queue]") {
  ds::thread_safe_queue<int> q;
  q.push(42);

  int value{};
  REQUIRE(q.try_pop(value));
  REQUIRE(value == 42);
}

TEST_CASE("try_pop on empty queue returns false", "[queue]") {
  ds::thread_safe_queue<int> q;
  int value{};
  REQUIRE_FALSE(q.try_pop(value));
}

TEST_CASE("try_pop optional version", "[queue]") {
  ds::thread_safe_queue<int> q;

  REQUIRE_FALSE(q.try_pop().has_value());

  q.push(42);
  auto result = q.try_pop();
  REQUIRE(result.has_value());
  REQUIRE(*result == 42);
}

TEST_CASE("empty and size", "[queue]") {
  ds::thread_safe_queue<int> q;
  REQUIRE(q.empty());
  REQUIRE(q.size() == 0);

  q.push(1);
  REQUIRE_FALSE(q.empty());
  REQUIRE(q.size() == 1);

  q.push(2);
  REQUIRE(q.size() == 2);

  int val{};
  q.try_pop(val);
  REQUIRE(q.size() == 1);
}

TEST_CASE("FIFO ordering", "[queue]") {
  ds::thread_safe_queue<int> q;
  q.push(1);
  q.push(2);
  q.push(3);

  int val{};
  q.try_pop(val);
  REQUIRE(val == 1);
  q.try_pop(val);
  REQUIRE(val == 2);
  q.try_pop(val);
  REQUIRE(val == 3);
}

TEST_CASE("emplace constructs in place", "[queue]") {
  ds::thread_safe_queue<std::pair<int, std::string>> q;
  q.emplace(42, "hello");

  auto result = q.try_pop();
  REQUIRE(result.has_value());
  REQUIRE(result->first == 42);
  REQUIRE(result->second == "hello");
}

// -------- MOVE SEMANTICS ---------

TEST_CASE("move-only types supported", "[queue][move]") {
  ds::thread_safe_queue<std::unique_ptr<int>> q;

  q.push(std::make_unique<int>(42));

  auto result = q.try_pop();
  REQUIRE(result.has_value());
  REQUIRE(**result == 42);
}

TEST_CASE("emplace with move-only types", "[queue][move]") {
  ds::thread_safe_queue<std::unique_ptr<int>> q;
  q.emplace(new int(42));

  std::unique_ptr<int> ptr;
  REQUIRE(q.try_pop(ptr));
  REQUIRE(*ptr == 42);
}

// ------- BLOCKING OPERATIONS --------

TEST_CASE("wait_and_pop blocks until element available", "[queue][blocking]") {
  ds::thread_safe_queue<int> q;
  std::atomic<bool> popped{false};

  std::thread consumer([&] {
    int val{};
    q.wait_and_pop(val);
    REQUIRE(val == 42);
    popped = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE_FALSE(popped);

  q.push(42);
  consumer.join();

  REQUIRE(popped);
}

TEST_CASE("wait_and_pop optional version blocks", "[queue][blocking]") {
  ds::thread_safe_queue<int> q;

  std::thread consumer([&] {
    auto result = q.wait_and_pop();
    REQUIRE(result.has_value());
    REQUIRE(*result == 99);
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  q.push(99);
  consumer.join();
}

TEST_CASE("wait_for times out on empty queue", "[queue][timeout]") {
  ds::thread_safe_queue<int> q;

  int val{};
  auto start = std::chrono::steady_clock::now();
  bool success = q.wait_for(val, std::chrono::milliseconds(100));
  auto elapsed = std::chrono::steady_clock::now() - start;

  REQUIRE_FALSE(success);
  REQUIRE(elapsed >= std::chrono::milliseconds(100));
  REQUIRE(elapsed < std::chrono::milliseconds(200));
}

TEST_CASE("wait_for returns immediately if element available",
          "[queue][timeout]") {
  ds::thread_safe_queue<int> q;
  q.push(42);

  auto start = std::chrono::steady_clock::now();
  auto result = q.wait_for(std::chrono::milliseconds(100));
  auto elapsed = std::chrono::steady_clock::now() - start;

  REQUIRE(result.has_value());
  REQUIRE(*result == 42);
  REQUIRE(elapsed < std::chrono::milliseconds(50));
}

// ---------- SHUTDOWN ----------

TEST_CASE("push throws after shutdown", "[queue][shutdown]") {
  ds::thread_safe_queue<int> q;
  q.shutdown();

  REQUIRE_THROWS_AS(q.push(42), std::runtime_error);
}

TEST_CASE("is_shutdown reflects state", "[queue][shutdown]") {
  ds::thread_safe_queue<int> q;
  REQUIRE_FALSE(q.is_shutdown());

  q.shutdown();
  REQUIRE(q.is_shutdown());
}

TEST_CASE("shutdown wakes blocked waiters", "[queue][shutdown]") {
  ds::thread_safe_queue<int> q;
  std::atomic<bool> returned{false};

  std::thread consumer([&] {
    int val{};
    bool success = q.wait_and_pop(val);
    REQUIRE_FALSE(success);
    returned = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  REQUIRE_FALSE(returned);

  q.shutdown();
  consumer.join();

  REQUIRE(returned);
}

TEST_CASE("can drain remaining items after shutdown", "[queue][shutdown]") {
  ds::thread_safe_queue<int> q;
  q.push(1);
  q.push(2);
  q.push(3);

  q.shutdown();

  int val{};
  REQUIRE(q.try_pop(val));
  REQUIRE(val == 1);
  REQUIRE(q.try_pop(val));
  REQUIRE(val == 2);
  REQUIRE(q.try_pop(val));
  REQUIRE(val == 3);
  REQUIRE_FALSE(q.try_pop(val));
}

TEST_CASE("wait_and_pop drains after shutdown", "[queue][shutdown]") {
  ds::thread_safe_queue<int> q;
  q.push(1);
  q.push(2);

  q.shutdown();

  auto r1 = q.wait_and_pop();
  auto r2 = q.wait_and_pop();
  auto r3 = q.wait_and_pop();

  REQUIRE(r1.has_value());
  REQUIRE(*r1 == 1);
  REQUIRE(r2.has_value());
  REQUIRE(*r2 == 2);
  REQUIRE_FALSE(r3.has_value());
}

// ----------- STRESS TESTS (MPMC) ----------

TEST_CASE("MPMC stress test", "[queue][stress]") {
  ds::thread_safe_queue<int> q;
  constexpr int num_producers = 4;
  constexpr int num_consumers = 4;
  constexpr int items_per_producer = 2500;
  constexpr int total_items = num_producers * items_per_producer;

  std::atomic<int> items_consumed{0};
  std::atomic<long long> sum_produced{0};
  std::atomic<long long> sum_consumed{0};

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  for (int i = 0; i < num_consumers; ++i) {
    consumers.emplace_back([&] {
      while (true) {
        int val{};
        if (q.wait_and_pop(val)) {
          sum_consumed += val;
          items_consumed++;
        } else {
          break;
        }
      }
    });
  }

  for (int i = 0; i < num_producers; ++i) {
    producers.emplace_back([&, i] {
      for (int j = 0; j < items_per_producer; ++j) {
        int val = i * items_per_producer + j;
        q.push(val);
        sum_produced += val;
      }
    });
  }

  for (auto &t : producers) {
    t.join();
  }

  while (items_consumed < total_items) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  q.shutdown();

  for (auto &t : consumers) {
    t.join();
  }

  REQUIRE(items_consumed == total_items);
  REQUIRE(sum_consumed == sum_produced);
}

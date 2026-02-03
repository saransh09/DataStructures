#include "latch.hpp"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

TEST_CASE("Test end2end flow") {
  const int NUM_WORKERS = 4;
  CountDownLatch init_latch(NUM_WORKERS);
  CountDownLatch start_latch(1);

  std::vector<std::thread> workers;

  for (int i = 0; i < NUM_WORKERS; i++) {
    workers.emplace_back([&, i] {
      std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 2)));
      std::cout << "Worker " << i << " initializing...\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 2)));
      std::cout << "Thread " << i << " ready\n";
      init_latch.count_down();
      start_latch.wait();
      std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 2)));
      std::cout << "Worker " << i << " working\n";
    });
    std::cout << "Main: waiting for workers to initialize...\n";
    init_latch.wait();

    std::cout << "Main: all workers ready, starting\n";
    start_latch.count_down();
    for (auto &t : workers) {
      if (t.joinable()) {
        t.join();
      }
    }
  }
}

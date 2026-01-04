#pragma once

#include "thread_safe_queue.hpp"

#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace ds {

class thread_pool {
public:
  // Create pool with specified number of threads (0 = hardware_concurrency)
  explicit thread_pool(size_t num_threads = 0);

  // Destructor - graceful shutdown
  ~thread_pool();

  // Non-copyable, non-movable
  thread_pool(const thread_pool &) = delete;
  thread_pool &operator=(const thread_pool &) = delete;
  thread_pool(thread_pool &&) = delete;
  thread_pool &operator=(thread_pool &&) = delete;

  // Submit a task and get a future for the result
  template <typename F, typename... Args>
  auto submit(F &&f, Args &&...args)
      -> std::future<std::invoke_result_t<F, Args...>>;

  // Wait for all queued tasks to complete (doesn't shutdown)
  void wait_all();

  // Graceful shutdown - finish pending tasks, then stop
  void shutdown();

  // Query state
  [[nodiscard]] size_t thread_count() const noexcept;
  [[nodiscard]] bool is_shutdown() const noexcept;

private:
  void worker_loop();

  thread_safe_queue<std::function<void()>> task_queue_;
  std::vector<std::thread> workers_;
  std::atomic<bool> shutdown_{false};
};

// ============================================================================
// Implementation
// ============================================================================

inline thread_pool::thread_pool(size_t num_threads) {
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency() - 1;
    if (num_threads == 0) {
      num_threads = 1; // Fallback if hardware_concurrency() returns 0
    }
  }

  workers_.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back([this] { worker_loop(); });
  }
}

inline thread_pool::~thread_pool() { shutdown(); }

inline void thread_pool::worker_loop() {
  while (true) {
    std::function<void()> task;
    if (!task_queue_.wait_and_pop(task)) {
      // Queue shutdown and empty
      break;
    }
    task();
  }
}

inline void thread_pool::shutdown() {
  if (shutdown_.exchange(true)) {
    return;
  }
  task_queue_.shutdown();
  for (auto &worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

inline void thread_pool::wait_all() {
  // Submit a "barrier" task for each worker and wait for all to complete
  std::vector<std::future<void>> barriers;
  barriers.reserve(workers_.size());

  for (size_t i = 0; i < workers_.size(); ++i) {
    barriers.push_back(submit([] {}));
  }

  for (auto &barrier : barriers) {
    barrier.wait();
  }
}

inline size_t thread_pool::thread_count() const noexcept {
  return workers_.size();
}

inline bool thread_pool::is_shutdown() const noexcept {
  return shutdown_.load();
}

template <typename F, typename... Args>
auto thread_pool::submit(F &&f, Args &&...args)
    -> std::future<std::invoke_result_t<F, Args...>> {
  using return_type = std::invoke_result_t<F, Args...>;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      [func = std::forward<F>(f),
       ... args = std::forward<Args>(args)]() mutable {
        return func(std::forward<Args>(args)...);
      });

  std::future<return_type> result = task->get_future();

  task_queue_.push([task]() { (*task)(); });

  return result;
}

} // namespace ds

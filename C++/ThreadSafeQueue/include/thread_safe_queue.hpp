#pragma once

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <utility>

namespace ds {

template <typename T> class thread_safe_queue {
public:
  thread_safe_queue() = default;
  ~thread_safe_queue();

  // Non copyable, non movable
  thread_safe_queue(const thread_safe_queue &) = delete;
  thread_safe_queue &operator=(const thread_safe_queue &) = delete;
  thread_safe_queue(thread_safe_queue &&) = delete;
  thread_safe_queue &operator=(thread_safe_queue &&) = delete;

  // ----- PRODUCER API ----
  void push(const T &value);
  void push(T &&value);

  template <typename... Args> void emplace(Args &&...args);

  // ----- CONSUMER API ----

  // Blocking wait (forever) - returns fale only on shutdown
  bool wait_and_pop(T &out);
  [[nodiscard]] std::optional<T> wait_and_pop();

  // Blocking wait with timeout - returns false on shutdown OR timeout
  template <typename Rep, typename Period>
  bool wait_for(T &out, std::chrono::duration<Rep, Period> timeout);

  template <typename Rep, typename Period>
  [[nodiscard]] std::optional<T>
  wait_for(std::chrono::duration<Rep, Period> timeout);

  // Non-blocking - returns false if empty
  bool try_pop(T &out);
  [[nodiscard]] std::optional<T> try_pop();

  // ------ LIFECYCLE -------
  void shutdown();
  [[nodiscard]] bool is_shutdown() const;

  // ------- CAPACITY -------
  [[nodiscard]] size_t size() const;
  [[nodiscard]] bool empty() const;

private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool shutdown_{false};
};

// Destructor Implementation

template <typename T> thread_safe_queue<T>::~thread_safe_queue() { shutdown(); }

// Lifecycle API Implementation

template <typename T> void thread_safe_queue<T>::shutdown() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_ = true;
  }
  cv_.notify_all();
}

template <typename T> bool thread_safe_queue<T>::is_shutdown() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return shutdown_;
}

// Capacity API implementation

template <typename T> size_t thread_safe_queue<T>::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

template <typename T> bool thread_safe_queue<T>::empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}

// Producer API Implementation

template <typename T> void thread_safe_queue<T>::push(const T &value) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (shutdown_) {
      throw std::runtime_error("push() called on shutdown queue");
    }
    queue_.push(value);
  }
  cv_.notify_one();
}

template <typename T> void thread_safe_queue<T>::push(T &&value) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (shutdown_) {
      throw std::runtime_error("push() called on shutdown queue");
    }
    queue_.push(std::move(value));
  }
  cv_.notify_one();
}

template <typename T>
template <typename... Args>
void thread_safe_queue<T>::emplace(Args &&...args) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (shutdown_) {
      throw std::runtime_error("emplace() called on shutdown queue");
    }
    queue_.emplace(std::forward<Args>(args)...);
  }
  cv_.notify_one();
}

// ------ CONSUMER API (non blocking) ------

template <typename T> bool thread_safe_queue<T>::try_pop(T &out) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) {
    return false;
  }
  out = std::move(queue_.front());
  queue_.pop();
  return true;
}

template <typename T> std::optional<T> thread_safe_queue<T>::try_pop() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) {
    return std::nullopt;
  }
  T value = std::move(queue_.front());
  queue_.pop();
  return value;
}

// ------- CONSUMER API (blocking) --------
template <typename T> bool thread_safe_queue<T>::wait_and_pop(T &out) {
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
  if (queue_.empty()) {
    return false;
  }
  out = std::move(queue_.front());
  queue_.pop();
  return true;
}

template <typename T> std::optional<T> thread_safe_queue<T>::wait_and_pop() {
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
  if (queue_.empty()) {
    return std::nullopt;
  }
  T value = std::move(queue_.front());
  queue_.pop();
  return value;
}

// --------- CONSUMER API (blocking with timeout) ------
template <typename T>
template <typename Rep, typename Period>
bool thread_safe_queue<T>::wait_for(
    T &out, std::chrono::duration<Rep, Period> timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool success = cv_.wait_for(lock, timeout,
                              [this] { return !queue_.empty() || shutdown_; });
  if (!success || queue_.empty()) {
    return false;
  }
  out = std::move(queue_.front());
  queue_.pop();
  return true;
}

template <typename T>
template <typename Rep, typename Period>
std::optional<T>
thread_safe_queue<T>::wait_for(std::chrono::duration<Rep, Period> timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool success = cv_.wait_for(lock, timeout,
                              [this] { return !queue_.empty() || shutdown_; });
  if (!success || queue_.empty()) {
    return std::nullopt;
  }
  T value = std::move(queue_.front());
  queue_.pop();
  return value;
}

} // namespace ds

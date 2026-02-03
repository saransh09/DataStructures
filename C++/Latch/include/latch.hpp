#include <condition_variable>
#include <cstddef>
#include <mutex>

class CountDownLatch {
public:
  explicit CountDownLatch(std::size_t count) : count_(count) {}

  void count_down() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ > 0) {
      if (--count_ == 0) {
        cv_.notify_all();
      }
    }
  }

  void count_down(std::size_t n) {
    std::lock_guard<std::mutex> lock(mutex_);
    count_ = count_ >= n ? count_ - n : 0;
    if (count_ == 0) {
      cv_.notify_all();
    }
  }

  void wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return count_ == 0; });
  }

  std::size_t get_count() {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
  }

  bool is_done() {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_ == 0;
  }

private:
  std::size_t count_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

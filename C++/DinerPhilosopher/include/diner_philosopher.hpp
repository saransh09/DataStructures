#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>


class DinerPhilosopher {
public:
  explicit DinerPhilosopher(int n) : n_(n), forks_(n), running_(true) {}

  ~DinerPhilosopher() { stop(); }

  DinerPhilosopher(const DinerPhilosopher &) = delete;

  DinerPhilosopher &operator=(const DinerPhilosopher &) = delete;

  void start() {
    if (!philosophers_.empty())
      return;
    running_ = true;
    for (int i = 0; i < n_; i++) {
      philosophers_.emplace_back(&DinerPhilosopher::philosopher, this, i);
    }
  }

  void stop() {
    running_ = false;
    for (auto &t : philosophers_) {
      if (t.joinable())
        t.join();
    }
    philosophers_.clear();
  }

  void run_for(std::chrono::milliseconds duration) {
    start();
    std::this_thread::sleep_for(duration);
    stop();
  }

private:
  void philosopher(int id) {
    std::mt19937 rng(id);
    std::uniform_int_distribution<int> dist(1, 10);
    int left = id;
    int right = (id + 1) % n_;

    int first = std::min(left, right);
    int second = std::max(left, right);

    int eat_count = 0;

    while (running_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
      while (running_) {
        if (forks_[first].try_lock_for(std::chrono::milliseconds(5))) {
          if (forks_[second].try_lock_for(std::chrono::milliseconds(5))) {
            break;
          }
          forks_[first].unlock();
        }
      }
      if (!running_)
        break;

      eat_count++;
      std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));

      forks_[first].unlock();
      forks_[second].unlock();
    }
    std::cout << "Philosopher " << id << " ate " << eat_count << " times\n";
  }

  int n_;
  std::vector<std::timed_mutex> forks_;
  std::vector<std::thread> philosophers_;
  std::atomic<bool> running_;
};

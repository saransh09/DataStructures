#pragma once
#include <algorithm>
#include <cstddef>
#include <future>
#include <iterator>
#include <numeric>
#include <thread>
#include <vector>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

using UL = unsigned long;

/*
 * Below is an implementation of parallel_accumulate
 * Discuss which of the following problems does the code suffer from?
 * A) Load imbalance
 * B) Sequential bottleneck
 * C) Cache contention
 * D) All of the above
 */

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
  auto length = static_cast<UL>(std::distance(first, last));
  if (!length)
    return init;

  UL num_threads = std::thread::hardware_concurrency();
  UL block_size = num_threads == 0 ? 1 : length / num_threads;

  std::vector<T> result(num_threads);
  std::vector<std::thread> threads(num_threads - 1);

  Iterator block_start = first;
  for (UL i = 0; i < (num_threads - 1); ++i) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);
    threads[i] = std::thread([=, &result]() {
      result[i] = std::accumulate(block_start, block_end, T{});
    });
    block_start = block_end;
  }
  result[num_threads - 1] = std::accumulate(block_start, last, T{});

  for (auto &t : threads)
    t.join();

  return std::accumulate(result.begin(), result.end(), init);
}

/*
 * The code suffers from all of the above problems
 * Load Imbalance : Some of the blocks can take longer to process
 * Cache Contention : Cache contention from threads writing to adjacent result
 * vector elements This causes false-sharing and the cache lines ping pong
 * between cores. Sequential bottleneck : The thread creating and final
 * accumulate are sequential operations
 */

/*
 * Some of the ways to alleviate these problems are
 */

/*
 * 1) Variant 1 — Fixed threads + padding (minimal fix, educational)
 * Cap thread count
 * Static partitioning (still imperfect)
 * Padding to avoid false sharing
 * Still has a mild sequential bottleneck
 */

template <typename T> struct alignas(64) Padded {
  T value{};
};

template <typename Iterator, typename T>
T parallel_accumulate_padded(Iterator first, Iterator last, T init) {
  auto length = static_cast<std::size_t>(std::distance(first, last));
  if (length == 0)
    return init;

  std::size_t num_threads = std::thread::hardware_concurrency();
  if (num_threads == 0)
    num_threads = 1;

  std::size_t block_size = length / num_threads;

  std::vector<Padded<T>> results(num_threads);
  std::vector<std::thread> threads(num_threads - 1);

  Iterator block_start = first;

  for (std::size_t i = 0; i < num_threads - 1; ++i) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);

    threads[i] =
        std::thread([begin = block_start, end = block_end, &results, i]() {
          results[i].value = std::accumulate(begin, end, T{});
        });
    block_start = block_end;
  }

  results[num_threads - 1].value = std::accumulate(block_start, last, T{});
  for (auto &t : threads)
    t.join();

  T total = init;
  for (auto &r : results)
    total += r.value;
  return total;
}

/*
 * Variant 2 : Thread pool + dynamic work stealing (industrial style)
 * This is how real systems work
 * Persistent threads
 * Dynamic task queue
 * Small work chunks
 * No thread creation bottleneck
 */

template <typename Iterator, typename T, typename Pool>
T parallel_accumulate_pool(Iterator first, Iterator last, T init, Pool &pool) {
  constexpr std::size_t chunk_size = 4096;

  std::vector<std::future<T>> futures;

  while (first != last) {
    Iterator chunk_end = first;
    std::advance(chunk_end,
                 std::min(chunk_size, std::size_t(std::distance(first, last))));
    futures.push_back(pool.submit([first, chunk_end]() {
      return std::accumulate(first, chunk_end, T{});
    }));
    first = chunk_end;
  }
  T total = init;
  for (auto &f : futures)
    total += f.get();
  return total;
}

/*
 * Variant 3 — Recursive divide-and-conquer
 * It achieves:
 * Perfect tree reduction
 * Natural load balancing
 * No shared memory writes
 */

template <typename Iterator, typename T>
T parallel_accumulate_dc(Iterator first, Iterator last, T init) {
  constexpr std::ptrdiff_t cutoff = 10'000;
  auto length = std::distance(first, last);
  if (length < cutoff)
    return std::accumulate(first, last, init);

  Iterator mid = first;
  std::advance(mid, length / 2);

  auto left =
      std::async(std::launch::async, [first, mid]() {
        return parallel_accumulate_dc(first, mid, T{});
      });
  T right = parallel_accumulate_dc(mid, last, T{});
  return init + left.get() + right;
}

/*
 * Variation 4 - Using TBB parallel_reduce
 * This is the proper way to do parallel reduction on macOS with libc++
 * since std::execution policies are not yet supported
*/


template<typename Iterator, typename T>
T parallel_accumulate_std(Iterator first, Iterator last, T init)
{
    auto length = std::distance(first, last);
    if (length == 0) return init;
    
    return tbb::parallel_reduce(
        tbb::blocked_range<Iterator>(first, last),
        init,
        [](const tbb::blocked_range<Iterator>& range, T partial) {
            return std::accumulate(range.begin(), range.end(), partial);
        },
        std::plus<T>{}
    );
}

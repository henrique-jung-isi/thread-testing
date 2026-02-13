#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
  template <class F, class... Args>
  using result_of = std::invoke_result_t<F, Args...>;

public:
  ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
  ~ThreadPool();

  template <class F, class... Args>
  std::future<result_of<F, Args...>> enqueue(F &&f, Args &&...args);

private:
  std::vector<std::thread> _threads;
  std::queue<std::function<void()>> _tasks;
  std::mutex _mutex;
  std::condition_variable _cv;
  bool _stop{false};
};

inline ThreadPool::ThreadPool(size_t num_threads) {
  for (size_t i = 0; i < num_threads; ++i) {
    _threads.emplace_back([this] {
      for (;;) {
        std::unique_lock lock(_mutex);

        _cv.wait(lock, [this] { return !_tasks.empty() || _stop; });

        if (_stop && _tasks.empty()) {
          return;
        }
        auto task = std::move(_tasks.front());
        _tasks.pop();
        lock.unlock();

        task();
      }
    });
  }
}

inline ThreadPool::~ThreadPool() {
  {
    std::lock_guard lock(_mutex);
    _stop = true;
  }
  _cv.notify_all();
  for (auto &thread : _threads) {
    thread.join();
  }
}

template <class F, class... Args>
inline std::future<std::invoke_result_t<F, Args...>>
ThreadPool::enqueue(F &&f, Args &&...args) {
  // Previous approach:
  // std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  auto tuple = std::make_tuple(std::forward<Args>(args)...);
  auto task = std::make_shared<std::packaged_task<result_of<F, Args...>()>>(
      [f = std::forward<F>(f), args = std::move(tuple)]() mutable {
        return std::apply(
            [&f](auto &&...xs) {
              return std::invoke(f, std::forward<decltype(xs)>(xs)...);
            },
            args);
      });

  auto future = task->get_future();
  {
    std::unique_lock lock(_mutex);
    if (_stop) {
      throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    _tasks.emplace([task]() { (*task)(); });
  }
  _cv.notify_one();
  return future;
}
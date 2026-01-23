#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using Operation = std::function<void()>;
using Promise = std::promise<void>;
using PromisePtr = std::unique_ptr<Promise>;
using Future = std::future<void>;

class ThreadPool {
public:
  ThreadPool(size_t num_threads = std::thread::hardware_concurrency());

  ~ThreadPool();

  Future enqueue(const Operation &operation);

private:
  struct Task {
    Operation operation;
    PromisePtr promise{std::make_unique<Promise>()};
  };
  std::vector<std::thread> _threads;
  std::queue<Task> _tasks;
  std::mutex _queue_mutex;
  std::condition_variable _cv;
  bool _stop{false};
};

ThreadPool::ThreadPool(size_t num_threads) {
  for (size_t i = 0; i < num_threads; ++i) {
    _threads.emplace_back([this] {
      while (true) {
        std::unique_lock lock(_queue_mutex);

        _cv.wait(lock, [this] { return !_tasks.empty() || _stop; });

        if (_stop && _tasks.empty()) {
          return;
        }
        auto [operation, promise] = std::move(_tasks.front());
        _tasks.pop();
        lock.unlock();

        operation();
        promise->set_value();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(_queue_mutex);
    _stop = true;
  }

  _cv.notify_all();

  for (auto &thread : _threads) {
    thread.join();
  }
}

Future ThreadPool::enqueue(const Operation &operation) {
  std::unique_lock lock(_queue_mutex);
  const auto &task = _tasks.emplace(std::move(Task{operation}));
  lock.unlock();

  auto future = task.promise->get_future();
  _cv.notify_one();
  return future;
}
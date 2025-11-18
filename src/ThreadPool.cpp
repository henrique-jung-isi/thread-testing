#include <ThreadPool.hpp>
#include <iostream>
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
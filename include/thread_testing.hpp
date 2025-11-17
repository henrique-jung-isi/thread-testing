#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using Operation = std::function<void()>;
using Promise = std::promise<void>;
using PromisePtr = std::unique_ptr<Promise>;
using Future = std::future<void>;

struct Task {
  Operation operation;
  PromisePtr promise{std::make_unique<Promise>()};
};

class ThreadTesting {
public:
  ThreadTesting(size_t num_threads = std::thread::hardware_concurrency());

  ~ThreadTesting();

  Future enqueue(const Operation &operation);

private:
  std::vector<std::thread> _threads;
  std::queue<Task> _tasks;
  std::mutex _queue_mutex;
  std::condition_variable _cv;
  bool _stop{false};
};
#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>

template <class T, class... Args> class AnotherPersistentThread {
public:
  AnotherPersistentThread(std::function<T(Args...)> &&operation);

  ~AnotherPersistentThread();

  std::future<T> enqueue(Args &&...args);

private:
  struct Task {
    Task(Args &&...args) : args(std::forward<Args>(args)...) {}
    std::tuple<Args...> args;
    std::unique_ptr<std::promise<T>> promisePtr{
        std::make_unique<std::promise<T>>()};
  };

  void runtime();

  std::function<T(Args...)> _operation;
  std::thread _thread;
  std::queue<Task> _tasks;
  std::mutex _cvMutex;
  std::condition_variable _cv;
  bool _stop{false};
};

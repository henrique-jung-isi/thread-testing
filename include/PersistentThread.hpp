#pragma once

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

template <class T, class... Args> class PersistentThread {
public:
  PersistentThread(std::function<T(Args...)> &&operation)
      : _operation{operation} {
    _thread = std::thread(&PersistentThread::runtime, this);
  };

  // Constructor for member function pointers
  template <class Class>
  PersistentThread(T (Class::*member_func)(Args...), Class *instance)
      : _operation{[member_func, instance](Args &&...args) {
          return (instance->*member_func)(std::forward<Args>(args)...);
        }} {
    _thread = std::thread(&PersistentThread::runtime, this);
  };

  ~PersistentThread() {
    {
      std::lock_guard lock(_cvMutex);
      _stop = true;
    }
    _cv.notify_all();
    _thread.join();
  };

  std::future<T> enqueue(Args &&...args) {
    std::unique_lock lock(_cvMutex);
    const auto &task = _tasks.emplace(std::forward<Args>(args)...);
    auto future = task.promisePtr->get_future();
    lock.unlock();
    _cv.notify_one();
    return future;
  };

private:
  struct Task {
    Task(Args &&...args) : args(std::forward<Args>(args)...) {}
    std::tuple<Args...> args;
    std::unique_ptr<std::promise<T>> promisePtr{
        std::make_unique<std::promise<T>>()};
  };

  void runtime() {
    while (true) {
      std::unique_lock lock(_cvMutex);
      _cv.wait(lock, [this] { return !_tasks.empty() || _stop; });
      if (_stop && _tasks.empty()) {
        return;
      }
      auto task = std::move(_tasks.front());
      _tasks.pop();
      lock.unlock();
      try {
        if constexpr (std::is_void_v<T>) {
          std::apply(_operation, task.args);
          task.promisePtr->set_value();
        } else {
          task.promisePtr->set_value(std::apply(_operation, task.args));
        }
      } catch (...) {
        task.promisePtr->set_exception(std::current_exception());
      }
    }
  };

  std::function<T(Args...)> _operation;
  std::thread _thread;
  std::queue<Task> _tasks;
  std::mutex _cvMutex;
  std::condition_variable _cv;
  bool _stop{false};
};
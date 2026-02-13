#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

template <class T, class... Args> class PersistentThread {
public:
  PersistentThread(std::function<T(Args...)> &&operation);

  // Constructor for member function pointers
  template <class Class>
  PersistentThread(T (Class::*member_func)(Args...), Class *instance);
  ~PersistentThread();

  std::future<T> enqueue(Args &&...args);

private:
  void runtime();

  std::function<T(Args...)> _operation;
  std::thread _thread;
  std::queue<std::function<void()>> _tasks;
  std::mutex _mutex;
  std::condition_variable _cv;
  bool _stop{false};
};

template <class T, class... Args>
inline PersistentThread<T, Args...>::PersistentThread(
    std::function<T(Args...)> &&operation)
    : _operation{operation} {
  _thread = std::thread(&PersistentThread::runtime, this);
}

template <class T, class... Args>
template <class Class>
inline PersistentThread<T, Args...>::PersistentThread(
    T (Class::*member_func)(Args...), Class *instance)
    : _operation{[member_func, instance](Args &&...args) {
        return (instance->*member_func)(std::forward<Args>(args)...);
      }} {
  _thread = std::thread(&PersistentThread::runtime, this);
}

template <class T, class... Args>
inline PersistentThread<T, Args...>::~PersistentThread() {
  {
    std::lock_guard lock(_mutex);
    _stop = true;
  }
  _cv.notify_all();
  _thread.join();
}

template <class T, class... Args>
inline std::future<T> PersistentThread<T, Args...>::enqueue(Args &&...args) {
  auto tuple = std::make_tuple(std::forward<Args>(args)...);
  auto task = std::make_shared<std::packaged_task<T()>>(
      [f = _operation, args = std::move(tuple)]() mutable {
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

template <class T, class... Args>
inline void PersistentThread<T, Args...>::runtime() {
  for (;;) {
    std::unique_lock lock(_mutex);

    _cv.wait(lock, [this] { return !_tasks.empty() || _stop; });

    if (_stop && _tasks.empty()) return;

    auto task = std::move(_tasks.front());
    _tasks.pop();
    lock.unlock();

    task();
  }
}

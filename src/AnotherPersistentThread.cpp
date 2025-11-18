#include <AnotherPersistentThread.hpp>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <class T, class... Args>
AnotherPersistentThread<T, Args...>::AnotherPersistentThread(
    std::function<T(Args...)> &&operation)
    : _operation{operation} {
  _thread = std::thread(&AnotherPersistentThread::runtime, this);
}

template <class T, class... Args>
AnotherPersistentThread<T, Args...>::~AnotherPersistentThread() {
  {
    std::lock_guard lock(_cvMutex);
    _stop = true;
  }
  _cv.notify_all();
  _thread.join();
}

template <class T, class... Args>
std::future<T> AnotherPersistentThread<T, Args...>::enqueue(Args &&...args) {
  std::unique_lock lock(_cvMutex);
  const auto &task = _tasks.emplace(std::forward<Args>(args)...);
  auto future = task.promisePtr->get_future();
  lock.unlock();
  _cv.notify_one();
  return future;
}

template <class T, class... Args>
void AnotherPersistentThread<T, Args...>::runtime() {
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
}
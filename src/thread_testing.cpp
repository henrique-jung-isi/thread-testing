#include <thread_testing.hpp>

ThreadTesting::ThreadTesting(size_t num_threads) {

  // Creating worker threads
  for (size_t i = 0; i < num_threads; ++i) {
    _threads.emplace_back([this] {
      while (true) {
        std::unique_lock<std::mutex> lock(_queue_mutex);

        _cv.wait(lock, [this] { return !_tasks.empty() || _stop; });

        if (_stop && _tasks.empty()) {
          return;
        }

        // Get the next task from the queue
        auto [operation, promise] = std::move(_tasks.front());
        _tasks.pop();
        lock.unlock();

        operation();
        promise->set_value();
      }
    });
  }
}

ThreadTesting::~ThreadTesting() {
  {
    // Lock the queue to update the stop flag safely
    std::unique_lock<std::mutex> lock(_queue_mutex);
    std::cout << "had " << _tasks.size() << "tasks" << std::endl;
    _stop = true;
  }

  // Notify all threads
  _cv.notify_all();

  // Joining all worker threads to ensure they have
  // completed their tasks
  for (auto &thread : _threads) {
    thread.join();
  }
}
Future ThreadTesting::enqueue(const Operation &operation) {
  std::unique_lock<std::mutex> lock(_queue_mutex);
  const auto &task = _tasks.emplace(std::move(Task{operation}));
  lock.unlock();

  auto future = task.promise->get_future();
  _cv.notify_one();
  return future;
}
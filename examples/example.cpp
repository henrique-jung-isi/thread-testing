#include <SimpleThreads/PersistentThread.hpp>
#include <SimpleThreads/ThreadPool.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <syncstream>
#include <vector>

using namespace std;

struct Example {
  string operation(double a, double b) {
    auto result = a * b;
    const auto id = this_thread::get_id();
    stringstream ss;
    ss << "From Example " << id << ": " << a << " * " << b << " = " << result
       << endl;
    return ss.str();
  }
  string operation2(double a, double b) {
    auto result = a * b;
    const auto id = this_thread::get_id();
    stringstream ss;
    ss << "From Example in Member function " << id << ": " << a << " * " << b
       << " = " << result << endl;
    return ss.str();
  }
};

string operation(double a, double b) {
  auto result = a * b;
  const auto id = this_thread::get_id();
  stringstream ss;
  ss << id << ": " << a << " * " << b << " = " << result << endl;
  return ss.str();
}

void poolOperation() {
  std::this_thread::sleep_for(1000ms);
  const auto id = this_thread::get_id();
  osyncstream(cout) << "thread " << id << " from pool:" << endl;
  for (auto i = 0; i < 10; i++) {
    osyncstream(cout) << "thread " << id << " - " << i << endl;
  }
}

int main(int argc, char *argv[]) {
  PersistentThread<string, double, double> thread(&operation);

  auto resultFuture = thread.enqueue(1.5, 2.5);
  auto resultFuture2 = thread.enqueue(1, 2.5);
  auto resultFuture3 = thread.enqueue(2.5, 2.5);
  osyncstream(cout) << resultFuture3.get();
  osyncstream(cout) << resultFuture.get();
  osyncstream(cout) << resultFuture2.get();

  Example e;
  ThreadPool pool(2);
  auto p = pool.enqueue(&poolOperation);
  auto p2 = pool.enqueue(&Example::operation, &e, 4, 5);
  auto p3 = pool.enqueue(&poolOperation);
  osyncstream(cout) << p2.get();
  p3.wait();

  p.wait();

  PersistentThread<string, double, double> memberThread(
      [&e](double a, double b) { return e.operation(a, b); });
  auto memberFuture = memberThread.enqueue(1.5, 2.5);
  osyncstream(cout) << memberFuture.get();

  PersistentThread<string, double, double> memberThread2(&Example::operation2,
                                                         &e);
  auto memberFuture2 = memberThread2.enqueue(1.5, 2.5);
  osyncstream(cout) << memberFuture2.get();

  return 0;
}
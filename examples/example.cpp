#include <PersistentThread.hpp>
#include <ThreadPool.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <syncstream>
#include <vector>

using namespace std;

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

  ThreadPool pool(2);
  auto p = pool.enqueue(&poolOperation);
  auto p3 = pool.enqueue(&poolOperation);
  p3.wait();
  p.wait();

  return 0;
}
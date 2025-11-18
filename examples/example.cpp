#include <ThreadPool.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <syncstream>
#include <vector>

using namespace std;
void poolOperation() {
  std::this_thread::sleep_for(1000ms);
  const auto id = this_thread::get_id();
  osyncstream(cout) << "thread " << id << " from pool:" << endl;
  for (auto i = 0; i < 10; i++) {
    osyncstream(cout) << "thread " << id << " - " << i << endl;
  }
}

int main(int argc, char *argv[]) {


  ThreadPool pool(2);
  auto p = pool.enqueue(&poolOperation);
  auto p3 = pool.enqueue(&poolOperation);
  p3.wait();
  p.wait();

  return 0;
}
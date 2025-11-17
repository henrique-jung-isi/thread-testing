#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread_testing.hpp>
#include <vector>
using namespace std;

struct Args {
  string path;
  bool showHelp{false};
  vector<string> error;
};

Args parseArgs(int argc, char *argv[]);
void showHelpMessage();

int main(int argc, char *argv[]) {
  const auto [path, showHelp, error] = parseArgs(argc, argv);
  ThreadTesting t1(2);
  ThreadTesting t2(2);

  auto p = t1.enqueue([]() {
    std::this_thread::sleep_for(1000ms);
    cout << "thread 1 from t1:" << endl;
    for (auto i = 0; i < 10; i++) {
      stringstream ss;
      ss << "thread 1- " << i << endl;
      cout << ss.str();
    }
  });

  auto p3 = t1.enqueue([]() {
    cout << "thread 2 from t1:" << endl;
    for (auto i = 0; i < 10; i++) {
      stringstream ss;
      ss << "thread t1-2- " << i << endl;
      cout << ss.str();
    }
  });
  auto p2 = t2.enqueue([]() {
    cout << "thread 1 from t2\n";
    // std::this_thread::sleep_for(1000ms);
    for (auto i = 0; i < 10; i++) {
      stringstream ss;
      ss << "thread 1- " << i << endl;
      cout << ss.str();
    }
  });
  p3.wait();

  p2.wait();
  p.wait();

  // if (!error.empty()) {
  //   cerr << "Unkown option: " << error[0] << endl;
  //   showHelpMessage();
  //   return 1;
  // }
  // if (showHelp) {
  //   showHelpMessage();
  //   return 0;
  // }

  return 0;
}

void showHelpMessage() {
  cout << "Usage:\n"
          "  example [OPTIONS] [INPUT]\n\n"

          "  This can be used to demonstrate project features.\n"

          "Input:\n"
          "  Some posicional input for the example \n\n"

          "Options:\n"
          "  -h, --help         Show help.\n"
          "  TODO: add remaining options.\n";
}

Args parseArgs(int argc, char *argv[]) {
  Args args;
  for (int i = 0; i < argc; i++) {
    const auto arg = string(argv[i]);
    if (arg == "-h" || arg == "--help") {
      args.showHelp = true;
    } else if (arg[0] == '-') {
      args.error.push_back(arg);
    } else {
      args.path = arg;
    }
  }
  return args;
}

# Simple Threads

This is an example of persistent threads and thread pool for c++.

## Why persistent?

Threads have a high creation cost, just placing a recurring process in a thread will sometimes lower the performance if the threads are created every time. With this library the threads are kept alive waiting for new information to process.

## Cloning and configuring repository

```{bash}
git clone https://github.com/henrique-jung-isi/SimpleThreads
cd SimpleThreads
mkdir build
cd build
cmake .. . -G Ninja 
# ou para release:
# cmake .. . -G Ninja -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build .
sudo cmake --install .
```

# Using the library in your own project

See [examples](examples/)

# Uninstallation

Use the target `uninstall`. In the root of the project run:

```{bash}
sudo cmake --build build --target uninstall
```

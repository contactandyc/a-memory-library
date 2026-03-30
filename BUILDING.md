# BUILDING

This project: **A Memory Library**
Version: **0.1.0**

## Local build

```bash
# one-shot build + install
./build.sh install
```

Or run the steps manually:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(nproc || sysctl -n hw.ncpu || echo 4)"
sudo cmake --install .
```



## Install dependencies (from `deps.libraries`)



### Development tooling (optional)

```bash
sudo apt-get update && sudo apt-get install -y valgrind gdb perl autoconf automake libtool python3 python3-venv python3-pip
```



### the-macro-library

Clone & build:

```bash
git clone --depth 1 --single-branch "https://github.com/contactandyc/the-macro-library.git" "the-macro-library"
cd "the-macro-library"
./build.sh clean
./build.sh install
cd ..
rm -rf "the-macro-library"
```


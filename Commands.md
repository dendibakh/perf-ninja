```bash
# Getting and building Google Bench
cd benchmark/
sudo cmake --build build --config Release --target install -j$(nproc)

# Linus perf install
sudo apt install linux-tools-common linux-tools-generic

# WSL2 perf install and build
sudo apt install -y pkg-config
sudo apt install -y flex bison
sudo apt install -y \
    libtraceevent-dev libtracefs-dev \
    libzstd-dev libcap-dev libelf-dev \
    libdw-dev libunwind-dev libnuma-dev \
    libssl-dev
git clone --depth 1 https://github.com/microsoft/WSL2-Linux-Kernel.git
cd WSL2-Linux-Kernel/tools/perf
make -j$(nproc)

cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..

perf stat --topdown -a taskset -c 0 ./lab

```
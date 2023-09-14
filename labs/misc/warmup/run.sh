set -e

cmake -B build
cmake --build build
build/lab
build/validate
#include <iostream>
#include <iterator>
#include <string>
#include <fstream>
#include "ao.h"

int ao_bench();

Sphere spheres[3];
Plane  plane;

int main(int argc, char **argv) {
  constexpr int mandatoryArgumentsCount = 1;
  if (argc != 1 + mandatoryArgumentsCount) {
    std::cerr << "Usage: /path/to/golden.ppm" << std::endl;
    return 1;
  }
  const std::string input = "ao.ppm";
  const std::string golden = argv[1];

  // generate ao.ppm
  ao_bench();
  
  // Simply compare two files byte-by-byte.
  // Not very efficient, but we don't care about the speed here.
  std::ifstream inputFile { input.c_str() }; 
	std::string inputContents {std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>()}; 
  std::ifstream goldenFile { golden}; 
	std::string goldenContents {std::istreambuf_iterator<char>(goldenFile), std::istreambuf_iterator<char>()}; 

  if (inputContents != goldenContents) {
    std::cerr << "Validation Failed. Images are not identical.\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}

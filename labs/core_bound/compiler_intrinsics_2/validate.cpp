
#include "solution.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

unsigned original_solution(const std::string &inputContents) {
  unsigned longestLine = 0;
  unsigned curLineLength = 0;
  for (auto s : inputContents) {
    if (s == '\n') {
      longestLine = std::max(curLineLength, longestLine);
      curLineLength = 0;
    } else {
      curLineLength++;
    }
  }
  // if no end-of-line in the end
  longestLine = std::max(curLineLength, longestLine);
  return longestLine;
}

int main(int argc, char **argv) {
  constexpr int mandatoryArgumentsCount = 1;
  if (argc != 1 + mandatoryArgumentsCount) {
    std::cerr << "Usage: lab path/to/inputs" << std::endl;
    return 1;
  }
  const std::string inputsDirName = argv[1];
  // feel free to comment out tests for debugging
  const std::vector<std::string> inputs = {
      "test1.txt",               // basic test
      "test2.txt",               // no end-of-line in the end
      "test3.txt",               // small number of characters
      "LoopVectorize.cpp",       // a large C++ file from the LLVM compiler.
      "MarkTwain-TomSawyer.txt", // a typical text file with long lines.
      "counter-example.txt"      // input where sequential solution is faster
  };

  bool valid = true;
  for (auto &input : inputs) {
    std::filesystem::path inputsDirPath = inputsDirName;
    inputsDirPath.append(input);
    std::ifstream inFile{inputsDirPath.string()};
    if (!inFile) {
      std::cerr << "Validation Failed. No input file: "
                << inputsDirPath.string() << "\n";
      return 1;
    }
    std::string inputContents{std::istreambuf_iterator<char>(inFile),
                              std::istreambuf_iterator<char>()};
    auto original_result = original_solution(inputContents);
    auto result = solution(inputContents);
    if (original_result != result) {
      std::cerr << "Validation Failed on test input: " << input
                << ". Original result = " << original_result
                << "; Modified version returned = " << result << "\n";
      valid = false;
    }
  }

  if (!valid)
    return 1;

  std::cout << "Validation Successful" << std::endl;
  return 0;
}

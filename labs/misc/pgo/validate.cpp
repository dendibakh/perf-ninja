
#include <iostream>
#include <fstream>

extern "C" { int runLuaBenchmark (int argc, char **argv); }

int main(int argc, char **argv) {
  constexpr int mandatoryArgumentsCount = 1;
  if (argc != 1 + mandatoryArgumentsCount) {
    std::cerr << "Usage: ./validate reference_output.txt" << std::endl;
    return 1;
  }
  const std::string outputFileName = "output.txt";
  
  // Run the scripts and save output into output.txt
  const char* run_validation_tests[] = { "lua", "../run_bench_tests.lua", "test", outputFileName.c_str(), 0 };
  int status = runLuaBenchmark(4/*argc*/, (char**)run_validation_tests);
  if (status) {
    std::cerr << "Error while running lua scripts. Validation failed.\n";
    return 1;
  }
  
  // Calculate the hash of output.txt and put it into output.txt.md5
  const char* check_md5[] = { "lua", "../md5/check_md5.lua", outputFileName.c_str(), 0 };
  status = runLuaBenchmark(3/*argc*/, (char**)check_md5);
  if (status) {
    std::cerr << "Error while running md5 lua script. Validation failed.\n";
    return 1;
  }

  // Compare output and reference hashes
  const std::string outputMd5FileName = "output.txt.md5";
  const std::string refOutputMd5FName = argv[1];

  std::ifstream outFile { outputMd5FileName }; 
	std::string outContents {std::istreambuf_iterator<char>(outFile), std::istreambuf_iterator<char>()}; 
	std::ifstream refFile { refOutputMd5FName }; 
	std::string refContents {std::istreambuf_iterator<char>(refFile), std::istreambuf_iterator<char>()}; 
  
  if (outContents != refContents) {
    std::cerr << "Validation Failed. Hashes do not match. Output hash = " << outContents << 
                 "; Reference hash = " << refContents << "\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}

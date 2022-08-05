# This script helps to check the speedup (and noise) of a benchmark
# by building and running a solution against the baseline code.

# Requirement: guard the code of the solution with #ifdef SOLUTION

# Example usage:
# $ mkdir check_speedup
# $ cd check_speedup
# $ python "C:\workspace\perf-ninja\tool\check_noise.py"
#   -lab_path "C:\workspace\perf-ninja\labs\misc\warmup"
#   -DSOLUTION -num_runs 3 -v

import subprocess
import os
import argparse
from pathlib import Path
import shutil
import gbench
from gbench import util, report
from gbench.util import *

parser = argparse.ArgumentParser(description='test results')
parser.add_argument("-num_runs", type=int, help="Number of runs", default=5)
parser.add_argument("-lab_path", type=str, help="Path to the root of the lab.")
parser.add_argument("-D", type=str, help="Path to the root of the lab.", default="SOLUTION")
parser.add_argument("-v", help="verbose", action="store_true", default=False)

args = parser.parse_args()
numRuns = args.num_runs
labRootPath = args.lab_path
defines = args.D
verbose = args.v

saveCWD = os.getcwd()

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def callWrapper(cmdLine):
  if verbose:
    print(bcolors.OKCYAN + "Running: " + cmdLine + bcolors.ENDC)
    subprocess.check_call(cmdLine, shell=True)
  else:
    subprocess.check_call(cmdLine, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
  return

def buildAndRunBench(iterNumber, variant, cmakeFlags):
  os.chdir(saveCWD)
  buildPath = os.path.join(saveCWD, variant + "Build" + str(iterNumber))
  buildDirPath = Path(buildPath)
  if buildDirPath.exists() and buildDirPath.is_dir():
    shutil.rmtree(buildDirPath)
  try:
    callWrapper("cmake -E make_directory " + variant +"Build" + str(iterNumber))
    os.chdir(buildPath)
    labAbsPath = labRootPath
    if not os.path.isabs(labAbsPath):
      labAbsPath = os.path.join(saveCWD, labRootPath)
    callWrapper("cmake -B . -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release " + cmakeFlags + " -S \"" + labAbsPath + "\"")
    callWrapper("cmake --build . --config Release --parallel 8")
    # this will save score in result.json file
    callWrapper("cmake --build . --config Release --target validateLab")
    callWrapper("cmake --build . --config Release --target benchmarkLab")
  except:
    print(bcolors.FAIL + variant + ": iteration " + str(iterNumber) + " - Failed" + bcolors.ENDC)
    return False
  print(bcolors.OKGREEN + variant + ": iteration " + str(iterNumber) + " - Done" + bcolors.ENDC)
  return True

def compareResults(iterNumber):
  baselineBuildPath = os.path.join(saveCWD, "baselineBuild" + str(iterNumber))
  baselineResultPath = os.path.join(baselineBuildPath, "result.json")
  solutionBuildPath = os.path.join(saveCWD, "solutionBuild" + str(iterNumber))
  solutionResultPath = os.path.join(solutionBuildPath, "result.json")

  outJsonSolution = gbench.util.load_benchmark_results(solutionResultPath)
  outJsonBaseline = gbench.util.load_benchmark_results(baselineResultPath)

  # Parse two report files and compare them
  diff_report = gbench.report.get_difference_report(
    outJsonBaseline, outJsonSolution, True)
  output_lines = gbench.report.print_difference_report(
    diff_report,
    False, True, 0.05, True)
  
  for ln in output_lines:
    print(ln)

  return True

for i in range(0, numRuns):
  buildAndRunBench(i, "baseline", "")
  buildAndRunBench(i, "solution", "-DCMAKE_CXX_FLAGS=-D" + defines)

os.chdir(saveCWD)

for i in range(0, numRuns):
  compareResults(i)

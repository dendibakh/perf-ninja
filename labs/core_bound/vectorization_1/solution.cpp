#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

#include <iostream>

#ifndef SOLUTION
#  define SOLUTION 2
#endif


#if SOLUTION == 0
#  pragma message("Baseline.")
#  include "solution_0.cpp"
#elif SOLUTION == 1
#  pragma message("My implementation.")
#  include "solution_1.cpp"
#elif SOLUTION == 2
#  pragma message("Facit.")
#  include "solution_2.cpp"
#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif


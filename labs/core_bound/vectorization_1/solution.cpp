#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

#include <iostream>

#ifndef SOLUTION
#  define SOLUTION 1
#endif


#if SOLUTION == 0
#  include "solution_0.cpp"
#elif SOLUTION == 1
#  include "solution_1.cpp"
#elif SOLUTION == 2
#  include "solution_2.cpp"
#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif


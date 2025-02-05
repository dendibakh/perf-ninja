
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

static bool compare(S const& a, S const& b) {
	return a.key1 == b.key1 ? a.key2 < b.key2 : a.key1 < b.key1;
}

void solution(std::array<S, N> &arr) {
	std::sort(arr.begin(), arr.end(), compare);
}

#include <algorithm>
#include <cmath>

#include "math.hpp"

bool is_close(const Tolerance_Context &tc, float a, float b) {
  return std::abs(a - b) <= std::max(tc.m_rel_tol * std::max(std::abs(a), std::abs(b)), tc.m_abs_tol);
}

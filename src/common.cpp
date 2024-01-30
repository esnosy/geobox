#include "common.hpp"

bool is_close(float a, float b) {
  // https://isocpp.org/wiki/faq/newbie#floating-point-arith
  constexpr float epsilon = 1e-5f;
  return std::abs(a - b) <= epsilon * std::abs(a);
}

bool is_close(const glm::vec3 &a, const glm::vec3 &b) {
  return is_close(a.x, b.x) && is_close(a.y, b.y) && is_close(a.z, b.z);
}

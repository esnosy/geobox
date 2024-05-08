#include <cassert>

#include <glm/glm.hpp>

#include "common.hpp"
#include "primitives.hpp"

const glm::vec3 &Triangle::operator[](int i) const {
  assert(i >= 0 && i <= 2);
  if (i == 0) return m_a;
  if (i == 1) return m_b;
  if (i == 2) return m_c;
  unreachable();
}

glm::vec3 &Triangle::operator[](int i) { return const_cast<glm::vec3 &>(static_cast<const Triangle &>(*this)[i]); }

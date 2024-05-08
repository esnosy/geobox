#pragma once

#include <glm/vec3.hpp>

struct Triangle {
  glm::vec3 m_a, m_b, m_c;
  const glm::vec3 &operator[](int i) const;
  glm::vec3 &operator[](int i);
};

struct Segment {
  glm::vec3 m_a, m_b;
};

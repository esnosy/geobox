#pragma once

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Point_Cloud_Object {
private:
  std::vector<glm::vec3> m_points;
  glm::mat4 m_model_matrix;
  unsigned int m_VAO;
  unsigned int m_VBO;

public:
  Point_Cloud_Object(const std::vector<glm::vec3> &points, const glm::mat4 &model_matrix);

  void draw() const;

  const std::vector<glm::vec3> &get_points() const { return m_points; }

  const glm::mat4 &get_model_matrix() const { return m_model_matrix; }
};

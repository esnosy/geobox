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
  // GPU memory is freed in destructor,
  // avoid double free by disabling copy constructor and copy assignment operator,
  // also known as the "Rule of three"
  Point_Cloud_Object(const Point_Cloud_Object &) = delete;
  Point_Cloud_Object &operator=(const Point_Cloud_Object &) = delete;
  ~Point_Cloud_Object();

  Point_Cloud_Object(const std::vector<glm::vec3> &points, const glm::mat4 &model_matrix);
  void draw() const;

  [[nodiscard]] const std::vector<glm::vec3> &get_points() const { return m_points; }

  [[nodiscard]] const glm::mat4 &get_model_matrix() const { return m_model_matrix; }
};

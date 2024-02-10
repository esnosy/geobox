#include <limits>

#include <glad/glad.h>

#include "geobox_exceptions.hpp"
#include "point_cloud_object.hpp"

Point_Cloud_Object::Point_Cloud_Object(const std::vector<glm::vec3> &points, const glm::mat4 &model_matrix)
    : m_points(points), m_model_matrix(model_matrix) {

  if (m_points.size() > std::numeric_limits<int>::max()) {
    throw Overflow_Check_Error(
        "Aborting point cloud object GPU mesh creation, too many points, TODO: support larger point clouds");
  }

  if (m_points.size() > (std::numeric_limits<unsigned int>::max() / sizeof(glm::vec3))) {
    throw Overflow_Check_Error(
        "Aborting point cloud object GPU mesh creation, too many points, TODO: support larger point clouds");
  }
  auto points_buffer_size = static_cast<unsigned int>(m_points.size() * sizeof(glm::vec3));

  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, points_buffer_size, m_points.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);
}

void Point_Cloud_Object::draw() const {
  glBindVertexArray(m_VAO);
  glDrawArrays(GL_POINTS, 0, static_cast<int>(m_points.size()));
}

Point_Cloud_Object::~Point_Cloud_Object() {
  glDeleteVertexArrays(1, &m_VAO);
  glDeleteBuffers(1, &m_VBO);
}

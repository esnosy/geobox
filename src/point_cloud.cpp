#include <iostream>
#include <limits>

#include <glad/glad.h>

#include "point_cloud.hpp"

std::shared_ptr<Point_Cloud> Point_Cloud_Creator::create_point_cloud() {
  return std::shared_ptr<Point_Cloud>(new Point_Cloud());
}

std::shared_ptr<Point_Cloud_Object>
Point_Cloud_Object_Creator::create_point_cloud_object(const std::shared_ptr<Point_Cloud> &point_cloud,
                                                      const glm::mat4 &model_matrix) {
  if (point_cloud->m_points.size() > std::numeric_limits<int>::max()) {
    std::cerr << "Aborting point cloud object GPU mesh creation, too many points, TODO: support larger point clouds"
              << std::endl;
    return {};
  }

  if (point_cloud->m_points.size() > (std::numeric_limits<unsigned int>::max() / sizeof(glm::vec3))) {
    std::cerr << "Aborting point cloud object GPU mesh creation, too many points, TODO: support larger point clouds"
              << std::endl;
    return {};
  }
  auto points_buffer_size = static_cast<unsigned int>(point_cloud->m_points.size() * sizeof(glm::vec3));

  std::shared_ptr<Point_Cloud_Object> result(new Point_Cloud_Object());
  result->m_point_cloud = point_cloud;
  result->m_model_matrix = model_matrix;

  glGenVertexArrays(1, &result->m_VAO);
  glBindVertexArray(result->m_VAO);

  glGenBuffers(1, &result->m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, result->m_VBO);
  glBufferData(GL_ARRAY_BUFFER, points_buffer_size, point_cloud->m_points.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);
  return result;
}

void Point_Cloud_Object::draw() const {
  glBindVertexArray(m_VAO);
  glDrawArrays(GL_POINTS, 0, static_cast<int>(m_point_cloud->get_points().size()));
}

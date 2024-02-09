#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Point_Cloud {
private:
  friend class Point_Cloud_Creator;
  friend class Point_Cloud_Object_Creator;

  std::vector<glm::vec3> m_points;
  // Private default constructor to only allow creating objects through public API
  Point_Cloud() = default;

public:
  void add_point(const glm::vec3 &p) { m_points.push_back(p); }

  const std::vector<glm::vec3> &get_points() const { return m_points; }
};

class Point_Cloud_Creator {
public:
  [[nodiscard]] static std::shared_ptr<Point_Cloud> create_point_cloud();
  Point_Cloud_Creator() = delete;
};

class Point_Cloud_Object {
private:
  friend class Point_Cloud_Object_Creator;

  std::shared_ptr<Point_Cloud> m_point_cloud;
  glm::mat4 m_model_matrix;
  unsigned int m_VAO;
  unsigned int m_VBO;
  // Private default constructor to only allow creating objects through public API
  Point_Cloud_Object() = default;

public:
  void draw() const;

  const std::shared_ptr<Point_Cloud> &get_point_cloud() const { return m_point_cloud; }

  const glm::mat4 &get_model_matrix() const { return m_model_matrix; }
};

class Point_Cloud_Object_Creator {
public:
  [[nodiscard]] static std::shared_ptr<Point_Cloud_Object>
  create_point_cloud_object(const std::shared_ptr<Point_Cloud> &point_cloud, const glm::mat4 &model_matrix);
  Point_Cloud_Object_Creator() = delete;
};

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Orbit_Camera {
private:
  glm::vec3 m_forward{};
  glm::vec3 m_camera_pos{};
  glm::vec3 m_orbit_sphere_tangent{};
  glm::vec3 m_orbit_sphere_bi_tangent{};
  glm::vec3 m_orbit_sphere_normal{};
  glm::mat4 m_view_matrix{};

public:
  float m_inclination;
  float m_azimuth;
  float m_orbit_radius;
  glm::vec3 m_orbit_origin;

  Orbit_Camera(float inclination, float azimuth, float orbit_radius, const glm::vec3 &orbit_origin);

  void update();

  [[nodiscard]] const glm::mat4 &get_view_matrix() const { return m_view_matrix; }

  [[nodiscard]] const glm::vec3 &get_right() const { return m_orbit_sphere_tangent; }

  [[nodiscard]] const glm::vec3 &get_up() const { return m_orbit_sphere_bi_tangent; }

  [[nodiscard]] const glm::vec3 &get_forward() const { return m_forward; }

  [[nodiscard]] const glm::vec3 &get_camera_pos() const { return m_camera_pos; }
};

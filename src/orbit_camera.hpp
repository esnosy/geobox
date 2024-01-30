#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Orbit_Camera {
private:
  float m_inclination;
  float m_azimuth;
  float m_orbit_radius;
  glm::vec3 m_orbit_origin;

  glm::vec3 m_camera_pos{};
  glm::vec3 m_orbit_sphere_tangent{};
  glm::vec3 m_orbit_sphere_bi_tangent{};
  glm::vec3 m_orbit_sphere_normal{};
  glm::mat4 m_view_matrix{};

public:
  Orbit_Camera(float inclination, float azimuth, float orbit_radius, glm::vec3 orbit_origin);
  void update(float inclination_offset, float azimuth_offset, float orbit_radius_offset, glm::vec3 orbit_origin_offset);

  [[nodiscard]] glm::mat4 get_view_matrix() const { return m_view_matrix; }

  [[nodiscard]] float get_orbit_radius() const { return m_orbit_radius; }

  [[nodiscard]] glm::vec3 get_right() const { return m_orbit_sphere_tangent; }

  [[nodiscard]] glm::vec3 get_up() const { return m_orbit_sphere_bi_tangent; }

  [[nodiscard]] glm::vec3 get_forward() const { return -1.0f * m_orbit_sphere_normal; }

  [[nodiscard]] glm::vec3 get_camera_pos() const { return m_camera_pos; }
};

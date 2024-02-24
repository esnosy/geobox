#include <glm/gtc/matrix_transform.hpp>

#include "orbit_camera.hpp"

void Orbit_Camera::update() {
  // Spherical coordinates unit vectors: https://mathworld.wolfram.com/SphericalCoordinates.html
  float sin_camera_inclination = glm::sin(m_inclination);
  float cos_camera_inclination = glm::cos(m_inclination);
  float sin_camera_azimuth = glm::sin(m_azimuth);
  float cos_camera_azimuth = glm::cos(m_azimuth);
  m_orbit_sphere_normal = {
      sin_camera_inclination * cos_camera_azimuth,
      sin_camera_inclination * sin_camera_azimuth,
      cos_camera_inclination,
  };
  m_orbit_sphere_tangent = {
      -1 * sin_camera_azimuth,
      cos_camera_azimuth,
      0,
  };
  m_orbit_sphere_bi_tangent = {
      cos_camera_inclination * cos_camera_azimuth,
      cos_camera_inclination * sin_camera_azimuth,
      -1 * sin_camera_inclination,
  };
  // We negate bi-tangent because inclination angle is relative to +Z axis, as inclination increases, Z value decreases,
  // and we want the bi-tangent to point to the direction of increasing Z instead
  m_orbit_sphere_bi_tangent *= -1;
  m_forward = -1.0f * m_orbit_sphere_normal;
  m_camera_pos = m_orbit_sphere_normal * m_orbit_radius + m_orbit_origin;
  glm::mat3 camera_basis(m_orbit_sphere_tangent, m_orbit_sphere_bi_tangent, m_orbit_sphere_normal);
  glm::mat4 camera(1.0f);
  camera = glm::translate(camera, m_camera_pos);
  camera = camera * glm::mat4(camera_basis);

  m_view_matrix = glm::inverse(camera);
}

Orbit_Camera::Orbit_Camera(float inclination, float azimuth, float orbit_radius, const glm::vec3 &orbit_origin)
    : m_inclination(inclination), m_azimuth(azimuth), m_orbit_radius(orbit_radius), m_orbit_origin(orbit_origin) {
  update();
}

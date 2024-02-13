#pragma once

#include <cmath> // for std::sqrt
#include <cstdint>
#include <memory> // for std::shared_ptr
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "indexed_triangle_mesh_object.hpp"
#include "orbit_camera.hpp"
#include "point_cloud_object.hpp"
#include "shader.hpp"

constexpr float DEFAULT_ORBIT_CAMERA_INCLINATION_RADIANS = 0.0f;
// Azimuth is relative to +X, so we can make default value -pi/2 (-90 degrees) to make the default camera right vector
// align with +X, since at 0 degrees the right vector aligns with +Y
constexpr float DEFAULT_ORBIT_CAMERA_AZIMUTH_RADIANS = -1.0f * glm::half_pi<float>();
constexpr float DEFAULT_ORBIT_CAMERA_RADIUS = 1.0f;
constexpr glm::vec3 DEFAULT_ORBIT_CAMERA_ORIGIN = glm::vec3(0.0f);

constexpr uint32_t DEFAULT_POINTS_ON_SURFACE_COUNT = 100;

class Perspective_Projection {
private:
  glm::mat4 m_matrix;
  float m_fov_degrees = 45.0f;
  float m_aspect_ratio = 1.0f;
  float m_near = 0.01f;
  float m_far = 1000.0f;
  bool m_needs_update = true;

  void update() { m_matrix = glm::perspective(glm::radians(m_fov_degrees), m_aspect_ratio, m_near, m_far); }

public:
  void set_fov_degrees(float fov_degrees) {
    m_fov_degrees = fov_degrees;
    m_needs_update = true;
  }

  float get_fov_degrees() const { return m_fov_degrees; };

  void set_dimensions(int width, int height) {
    m_aspect_ratio = (float)width / (float)height;
    m_needs_update = true;
  }

  void set_near(float near) {
    m_near = near;
    m_needs_update = true;
  }

  float get_near() const { return m_near; }

  void set_far(float far) {
    m_far = far;
    m_needs_update = true;
  }

  float get_far() const { return m_far; }

  const glm::mat4 &get_matrix() {
    if (m_needs_update) {
      update();
      m_needs_update = false;
    }
    return m_matrix;
  }
};

class GeoBox_App {
public:
  GeoBox_App();
  void main_loop();

  // Prevent copy
  GeoBox_App(const GeoBox_App &) = delete;
  GeoBox_App &operator=(const GeoBox_App &) = delete;

private:
  GLFWwindow *m_window = nullptr;

  std::shared_ptr<Shader> m_phong_shader;
  std::shared_ptr<Shader> m_point_cloud_shader;

  std::vector<std::shared_ptr<Indexed_Triangle_Mesh_Object>> m_objects;
  std::vector<std::shared_ptr<Point_Cloud_Object>> m_point_cloud_objects;

  Perspective_Projection m_perspective_projection;

  Orbit_Camera m_camera{DEFAULT_ORBIT_CAMERA_INCLINATION_RADIANS, DEFAULT_ORBIT_CAMERA_AZIMUTH_RADIANS,
                        DEFAULT_ORBIT_CAMERA_RADIUS, DEFAULT_ORBIT_CAMERA_ORIGIN};

  float m_delta_time = 0.0f;      // Time between current frame and last frame
  float m_last_frame_time = 0.0f; // Time of last frame

  std::optional<glm::vec2> m_last_mouse_pos;

  // Randomness
  std::random_device m_random_device;
  std::default_random_engine m_random_engine{m_random_device()};
  std::uniform_real_distribution<float> m_random_factor_uniform_distribution{0.0f, 1.0f};

  float get_random_factor() { return m_random_factor_uniform_distribution(m_random_engine); }

  // https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#SamplingaTriangle
  glm::vec2 random_triangle_barycentric_coords() {
    float u0 = get_random_factor();
    float u1 = get_random_factor();
    float su0 = std::sqrt(u0);
    return glm::vec2(1 - su0, u1 * su0);
  }

  // Init
  void init_glfw();
  static void init_glad();
  void init_imgui();
  void init_glfw_callbacks();
  bool init_shaders();

  // Callbacks
  friend void framebuffer_size_callback(GLFWwindow *window, int width, int height);

  // Main-loop internals
  void process_input();
  void render();
  static void shutdown();

  // Dialogs
  void on_load_stl_dialog_ok(const std::string &file_path);

  // Operations
  // Points on surface
  uint32_t m_points_on_surface_count = DEFAULT_POINTS_ON_SURFACE_COUNT;
  [[nodiscard]] std::vector<glm::vec3> generate_points_on_surface();
  void on_generate_points_on_surface_button_click();
};

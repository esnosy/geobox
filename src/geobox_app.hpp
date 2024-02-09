#pragma once

#include <optional>
#include <random>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "object.hpp"
#include "orbit_camera.hpp"
#include "point_cloud.hpp"

constexpr float DEFAULT_ORBIT_CAMERA_INCLINATION = 0.0f;
// Azimuth is relative to +X, so we can make default value -pi/2 (-90 degrees) to make the default camera right vector
// align with +X, since at 0 degrees the right vector aligns with +Y
constexpr float DEFAULT_ORBIT_CAMERA_AZIMUTH = -1.0f * glm::half_pi<float>();
constexpr float DEFAULT_ORBIT_CAMERA_RADIUS = 1.0f;
constexpr glm::vec3 DEFAULT_ORBIT_CAMERA_ORIGIN = glm::vec3(0.0f);

class GeoBox_App {
public:
  GeoBox_App();
  void main_loop();

  // Prevent copy
  GeoBox_App(const GeoBox_App &) = delete;
  GeoBox_App &operator=(const GeoBox_App &) = delete;

private:
  GLFWwindow *m_window = nullptr;

  unsigned int m_default_shader_program = 0;
  unsigned int m_point_cloud_shader_program = 0;

  std::vector<std::shared_ptr<Mesh_Object>> m_objects;
  std::vector<Point_Cloud_Object> m_point_cloud_objects;

  float m_perspective_fov_degrees = 45.0f;

  Orbit_Camera m_camera = Orbit_Camera(DEFAULT_ORBIT_CAMERA_INCLINATION, DEFAULT_ORBIT_CAMERA_AZIMUTH,
                                       DEFAULT_ORBIT_CAMERA_RADIUS, DEFAULT_ORBIT_CAMERA_ORIGIN);

  float m_delta_time = 0.0f;      // Time between current frame and last frame
  float m_last_frame_time = 0.0f; // Time of last frame

  std::optional<glm::vec2> m_last_mouse_pos;

  // Randomness
  std::random_device m_random_device;
  std::default_random_engine m_random_engine = std::default_random_engine(m_random_device());

  // Init
  void init_glfw();
  static void init_glad();
  void init_imgui();
  void init_glfw_callbacks();
  bool init_shaders();

  // Callbacks
  friend void window_refresh_callback(GLFWwindow *window);
  friend void framebuffer_size_callback(const GLFWwindow *window, int width, int height);

  // Main-loop internals
  void process_input();
  void render();
  static void shutdown();

  // Dialogs
  void on_load_stl_dialog_ok(const std::string &file_path);

  // Operations
  void generate_points_on_surface();
};

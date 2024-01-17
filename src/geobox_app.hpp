#pragma once

#include <optional>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GeoBox_App {
public:
  GeoBox_App();
  void main_loop();

private:
  GLFWwindow *m_window = nullptr;

  unsigned int m_default_shader_program = 0;

  // TODO: support multiple objects once we settle on a design
  bool m_is_object_loaded = false;

  // GPU Mesh
  unsigned int m_VAO = 0;
  // We only need VAO for drawing, but we also store VBO and EBO to update them and free them later
  // VAO references VBO and EBO so updates will be reflected when VAO is bound again
  unsigned int m_VBO = 0;
  unsigned int m_EBO = 0;
  int m_num_indices = 0;

  // CPU Mesh
  std::vector<glm::vec3> m_vertices;
  std::vector<unsigned int> m_indices;
  std::vector<glm::vec3> m_vertex_normals;

  float m_camera_fov_degrees = 45.0f;

  // Orbit camera
  float m_camera_inclination = 0.0f;
  float m_camera_azimuth = 0.0f;
  float m_camera_orbit_radius = 1.0f;

  float m_delta_time = 0.0f;      // Time between current frame and last frame
  float m_last_frame_time = 0.0f; // Time of last frame

  bool m_is_left_mouse_down = false;
  std::optional<glm::vec2> m_last_mouse_pos;

  // Init
  void init_glfw();
  static void init_glad();
  void init_imgui();
  void init_glfw_callbacks();
  bool init_shaders();

  // Callbacks
  friend void window_refresh_callback(GLFWwindow *window);
  friend void framebuffer_size_callback(const GLFWwindow *window, int width, int height);
  friend void cursor_pos_callback(GLFWwindow *window, double x_pos, double y_pos);
  friend void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
  friend void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

  // Main-loop internals
  void process_input();
  void render();
  static void shutdown();

  // Dialogs
  void on_load_stl_dialog_ok(const std::string &file_path);
};

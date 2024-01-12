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

  unsigned int m_default_shader_program;

  // TODO: support more objects once we settle on a design
  bool m_is_object_loaded = false;

  unsigned int m_VAO;
  // We only need VAO for drawing, but we also store VBO and EBO to update them and free them later
  // VAO references VBO and EBO so updates will be reflected when VAO is bound again
  unsigned int m_VBO;
  unsigned int m_EBO;

  std::vector<glm::vec3> m_vertices;
  std::vector<unsigned int> m_indices;

  glm::vec3 m_camera_pos = {0, 0, 3};
  glm::vec3 m_camera_front = {0, 0, -1};
  glm::vec3 m_up = {0, 1, 0};

  // TODO: group camera properties and refactor camera and optionally write better camera movement logic
  float m_camera_yaw = -90.0f, m_camera_pitch = 0.0f;
  float m_camera_fov_degrees = 45.0f;

  float m_delta_time = 0.0f;      // Time between current frame and last frame
  float m_last_frame_time = 0.0f; // Time of last frame

  bool m_is_left_mouse_down = false;
  std::optional<glm::vec2> m_last_mouse_pos;

  void init_glfw();
  void init_glad() const;
  void init_imgui();
  void init_glfw_callbacks();
  bool init_shaders();

  void window_refresh_callback(GLFWwindow *window);
  void framebuffer_size_callback(const GLFWwindow *window, int width, int height) const;
  void cursor_pos_callback(GLFWwindow *window, double x_pos, double y_pos);
  void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

  // Main-loop internals
  void process_input();
  void render();
  void shutdown() const;
  // Dialogs
  void on_load_stl_dialog_ok(const std::string &file_path);
  void scroll_callback(GLFWwindow *w, double x_offset, double y_offset);
};

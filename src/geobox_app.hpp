#pragma once

#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class GeoBox_App {
public:
  GeoBox_App();
  void main_loop();

private:
  GLFWwindow *m_window = nullptr;

  unsigned m_default_shader_program;

  // TODO: support more objects once we settle on a design
  bool is_object_loaded = false;

  unsigned int m_VAO;
  // We only need VAO for drawing, but we also store VBO and EBO to update them and free them later
  // VAO references VBO and EBO so updates will be reflected when VAO is bound again
  unsigned int m_VBO;
  // unsigned int m_EBO;

  std::vector<glm::vec3> m_vertices;
  // std::vector<unsigned int[3]> m_triangles;

  void init_glfw();
  void init_glad() const;
  void init_imgui();
  void init_glfw_callbacks();
  bool init_shaders();

  void window_refresh_callback(GLFWwindow *window);
  void framebuffer_size_callback(const GLFWwindow *window, int width, int height) const;

  // Main-loop internals
  void process_input();
  void render();
  void shutdown() const;

  // Dialogs
  void on_load_stl_dialog_ok(const std::string &file_path);
};

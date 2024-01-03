#pragma once

#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

struct GPU_Indexed_Triangle_Mesh {
  unsigned int m_VAO;
  unsigned int m_VBO;
  unsigned int m_EBO;
  unsigned int m_num_vertices;
  unsigned int m_num_indices;

  void draw() const {
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, 0);
  }
};

struct GPU_Triangle_Mesh {
  unsigned int m_VAO;
  unsigned int m_VBO;
  unsigned int m_num_vertices;

  void draw() const {
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_num_vertices);
  }
};

class GeoBox_App {
public:
  GeoBox_App();
  void main_loop();

private:
  GLFWwindow *m_window = nullptr;
  std::vector<GPU_Triangle_Mesh> m_gpu_meshes;
  std::vector<GPU_Indexed_Triangle_Mesh> m_gpu_indexed_meshes;
  unsigned m_default_shader_program;

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

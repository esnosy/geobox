#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "bvh.hpp"
#include "triangle.hpp"

class Indexed_Triangle_Mesh_Object {
private:
  // GPU Mesh
  unsigned int m_VAO = 0;
  // We only need VAO for drawing, but we also store VBO and EBO to update them and free them later
  // VAO references VBO and EBO so updates will be reflected when VAO is bound again
  unsigned int m_vertex_positions_buffer_object = 0;
  unsigned int m_vertex_normals_buffer_object = 0;
  unsigned int m_EBO = 0;
  int m_num_indices = 0;

  // CPU Mesh
  std::vector<glm::vec3> m_vertices;
  std::vector<unsigned int> m_indices;
  std::vector<glm::vec3> m_vertex_normals;
  std::vector<float> m_triangle_areas;

  glm::mat4 m_model_matrix{1.0f};
  glm::mat3 m_normal_matrix{1.0f};

  std::shared_ptr<BVH> m_triangles_bvh;

public:
  // GPU memory is freed in destructor,
  // avoid double free by disabling copy constructor and copy assignment operator,
  // also known as the "Rule of three"
  Indexed_Triangle_Mesh_Object(const Indexed_Triangle_Mesh_Object &) = delete;
  Indexed_Triangle_Mesh_Object &operator=(const Indexed_Triangle_Mesh_Object &) = delete;
  ~Indexed_Triangle_Mesh_Object();

  Indexed_Triangle_Mesh_Object(const std::vector<Triangle> &triangles, const glm::mat4 &model_matrix);
  void draw() const;

  [[nodiscard]] const glm::mat4 &get_model_matrix() const { return m_model_matrix; }

  [[nodiscard]] const glm::mat3 &get_normal_matrix() const { return m_normal_matrix; }

  [[nodiscard]] const std::vector<glm::vec3> &get_vertices() const { return m_vertices; }

  [[nodiscard]] const std::vector<unsigned int> &get_indices() const { return m_indices; }

  [[nodiscard]] const std::vector<float> &get_triangle_areas() const { return m_triangle_areas; }
};

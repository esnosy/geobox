#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "bvh.hpp"
#include "triangle.hpp"

class Object {
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

  glm::mat4 m_model;
  glm::mat3 m_normal;

  std::shared_ptr<BVH> m_triangles_bvh;

public:
  // GPU memory is freed in destructor,
  // avoid double free by disabling copy constructor and copy assignment operator,
  // also known as the "Rule of three"
  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;
  ~Object();

  Object(const std::vector<Triangle> &triangles, const glm::mat4 &model);
  void draw() const;

  [[nodiscard]] glm::mat4 get_model() const { return m_model; }

  [[nodiscard]] glm::mat3 get_normal() const { return m_normal; }

  [[nodiscard]] const std::vector<glm::vec3> &get_vertices() const { return m_vertices; }

  [[nodiscard]] const std::vector<unsigned int> &get_indices() const { return m_indices; }
};

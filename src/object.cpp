#include <cassert>
#include <iostream>

#include <glad/glad.h>
#include <glm/gtx/norm.hpp>

#include "bvh.hpp"
#include "object.hpp"

glm::vec3 closest_point_on_aabb(const glm::vec3 &point, BVH::Node::AABB const &aabb) {
  return glm::clamp(point, aabb.min, aabb.max);
}

float point_aabb_distance_squared(const glm::vec3 &point, BVH::Node::AABB const &aabb) {
  return glm::distance2(point, closest_point_on_aabb(point, aabb));
}

struct Sphere {
  glm::vec3 center;
  float radius;
};

bool sphere_aabb_intersection(const Sphere &sphere, BVH::Node::AABB const &aabb) {
  return point_aabb_distance_squared(sphere.center, aabb) <= (sphere.radius * sphere.radius);
}

std::shared_ptr<Object> Object::from_triangles(const std::vector<glm::vec3> &triangle_mesh_vertices,
                                               const glm::mat4 &model) {
  if (triangle_mesh_vertices.empty()) {
    std::cerr << "Empty mesh" << std::endl;
    return {};
  }
  if ((triangle_mesh_vertices.size() % 3) != 0) {
    std::cerr << "Vertices do not make up a whole number of triangles" << std::endl;
    return {};
  }

  // We cannot use std::make_shared directly if constructor is private
  auto result = std::shared_ptr<Object>(new Object());
  result->m_model = model;
  result->m_normal = glm::transpose(glm::inverse(model));

  std::vector<BVH::Node::AABB> vertices_as_bounding_boxes;
  vertices_as_bounding_boxes.reserve(triangle_mesh_vertices.size());
  for (const glm::vec3 &v : triangle_mesh_vertices) {
    BVH::Node::AABB aabb;
    aabb.min = v;
    aabb.max = v;
    vertices_as_bounding_boxes.push_back(aabb);
  }

  BVH bvh(vertices_as_bounding_boxes);
  if (bvh.did_build_fail()) {
    std::cerr << "Failed to build BVH" << std::endl;
    return {};
  }
  assert(bvh.count_primitives() == triangle_mesh_vertices.size());
  std::cout << "Num nodes = " << bvh.count_nodes() << std::endl;
  std::cout << "Max node size = " << bvh.calc_max_leaf_size() << std::endl;

  std::vector<unsigned int> indices(triangle_mesh_vertices.size());
  std::vector<bool> is_remapped_vec(triangle_mesh_vertices.size(), false);
  std::vector<glm::vec3> unique_vertices;
  unique_vertices.reserve(triangle_mesh_vertices.size());

  unsigned int final_unique_vertex_index = 0;
  for (unsigned int original_unique_vertex_index = 0; original_unique_vertex_index < triangle_mesh_vertices.size();
       original_unique_vertex_index++) {
    // Skip already remapped vertex
    if (is_remapped_vec[original_unique_vertex_index])
      continue;
    const glm::vec3 &unique_vertex = triangle_mesh_vertices[original_unique_vertex_index];
    constexpr float range = 0.0001f;
    Sphere sphere{.center = unique_vertex, .radius = range};

    auto duplicate_vertex_callback = [final_unique_vertex_index, &is_remapped_vec,
                                      &indices](unsigned int duplicate_vertex_index) {
      if (!is_remapped_vec[duplicate_vertex_index]) {
        indices[duplicate_vertex_index] = final_unique_vertex_index;
        is_remapped_vec[duplicate_vertex_index] = true;
      }
    };
    auto aabb_filter = [&const_sphere = std::as_const(sphere)](BVH::Node::AABB const &aabb) {
      return sphere_aabb_intersection(const_sphere, aabb);
    };
    auto primitive_filter = [&const_vertices = std::as_const(triangle_mesh_vertices),
                             &unique_vertex](unsigned int potential_duplicate_vertex_index) {
      return glm::distance2(unique_vertex, const_vertices[potential_duplicate_vertex_index]) <= (range * range);
    };
    bvh.foreach_primitive(duplicate_vertex_callback, aabb_filter, primitive_filter);

    final_unique_vertex_index++;
    unique_vertices.push_back(unique_vertex);
  }
  unique_vertices.shrink_to_fit();

  std::cout << "Num unique vertices = " << unique_vertices.size() << std::endl;

  // Pre-calculate number of triangles per vertex (can be used later for weighting normals)
  std::vector<float> num_triangles_per_vertex(unique_vertices.size(), 0.0f);
  for (unsigned int vi : indices) {
    if (num_triangles_per_vertex[vi] == std::numeric_limits<float>::max()) {
      std::cerr
          << "Number of triangles per vertex exceeds maximum value of a float, aborting vertex normals calculation"
          << std::endl
          << "Failed to load mesh" << std::endl;
      return {};
    }
    num_triangles_per_vertex[vi] += 1;
  }

  std::vector<glm::vec3> vertex_normals(unique_vertices.size(), glm::vec3(0.0f));
  for (unsigned int i = 0; i < indices.size(); i += 3) {
    const glm::vec3 &a = unique_vertices[indices[i + 0]];
    const glm::vec3 &b = unique_vertices[indices[i + 1]];
    const glm::vec3 &c = unique_vertices[indices[i + 2]];
    glm::vec3 triangle_normal = glm::normalize(glm::cross(b - a, c - a));
    for (int j = 0; j < 3; j++) {
      unsigned int vi = indices[i + j];
      // Avoid overflow by dividing values while accumulating them
      vertex_normals[vi] += triangle_normal / num_triangles_per_vertex[vi];
    }
  }
  // Ensure normalized normals, since weighted sum of normals can not be guaranteed to be normalized
  // (some normals can be zero, weights might not sum up to 1.0f, etc...)
  for (glm::vec3 &vertex_normal : vertex_normals) {
    vertex_normal = glm::normalize(vertex_normal);
  }

  if (unique_vertices.size() > (std::numeric_limits<unsigned int>::max() / sizeof(glm::vec3))) {
    std::cerr << "Aborting GPU mesh creation, too many vertices, TODO: support larger meshes" << std::endl;
    return {};
  }
  auto unique_vertices_buffer_size = static_cast<unsigned int>(unique_vertices.size() * sizeof(glm::vec3));

  if (indices.size() > (std::numeric_limits<int>::max() / sizeof(unsigned int))) {
    std::cerr << "Aborting GPU mesh creation, too many indices, TODO: support larger meshes" << std::endl;
    return {};
  }

  // Set CPU mesh
  result->m_vertices = unique_vertices;
  result->m_indices = indices;
  result->m_vertex_normals = vertex_normals;

  // Create new GPU mesh data
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int vertex_positions_buffer_object;
  glGenBuffers(1, &vertex_positions_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_positions_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, unique_vertices_buffer_size, unique_vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);

  result->m_num_indices = static_cast<int>(indices.size());
  int indices_buffer_size = result->m_num_indices * static_cast<int>(sizeof(unsigned int));
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_size, indices.data(), GL_STATIC_DRAW);

  // Vertex normals buffer has same size as vertex positions buffer if calculated properly
  unsigned int vertex_normals_buffer_size = unique_vertices_buffer_size;
  unsigned int vertex_normals_buffer_object;
  glGenBuffers(1, &vertex_normals_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_normals_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, vertex_normals_buffer_size, result->m_vertex_normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(1);

  result->m_VAO = VAO;
  result->m_vertex_positions_buffer_object = vertex_positions_buffer_object;
  result->m_vertex_normals_buffer_object = vertex_normals_buffer_object;
  result->m_EBO = EBO;

  // Build BVH for triangles
  std::vector<BVH::Node::AABB> triangle_bounding_boxes;
  for (unsigned int i = 0; i < indices.size(); i += 3) {
    const glm::vec3 &a = unique_vertices[indices[i + 0]];
    const glm::vec3 &b = unique_vertices[indices[i + 1]];
    const glm::vec3 &c = unique_vertices[indices[i + 2]];
    BVH::Node::AABB aabb;
    aabb.min = glm::min(a, glm::min(b, c));
    aabb.max = glm::max(a, glm::max(b, c));
    triangle_bounding_boxes.push_back(aabb);
  }
  result->m_triangles_bvh = std::make_unique<BVH>(triangle_bounding_boxes);

  return result;
}

void Object::draw() const {
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, nullptr);
}

Object::~Object() {
  glDeleteVertexArrays(1, &m_VAO);
  glDeleteBuffers(1, &m_vertex_positions_buffer_object);
  glDeleteBuffers(1, &m_vertex_normals_buffer_object);
  glDeleteBuffers(1, &m_EBO);
}

#include <cassert>
#include <cstdlib> // for std::div
#include <iostream>
#include <memory>  // for std::make_shared
#include <utility> // for std::as_const

#include <glad/glad.h>
#include <glm/gtx/norm.hpp>

#include "bvh.hpp"
#include "geobox_exceptions.hpp"
#include "indexed_triangle_mesh_object.hpp"
#include "triangle.hpp"

[[nodiscard]] static glm::vec3 closest_point_in_aabb(const glm::vec3 &point, const AABB &aabb) {
  return glm::clamp(point, aabb.min, aabb.max);
}

[[nodiscard]] static float point_aabb_distance_squared(const glm::vec3 &point, const AABB &aabb) {
  return glm::distance2(point, closest_point_in_aabb(point, aabb));
}

struct Sphere {
  glm::vec3 center;
  float radius;
};

[[nodiscard]] static bool sphere_aabb_intersection(const Sphere &sphere, const AABB &aabb) {
  return point_aabb_distance_squared(sphere.center, aabb) <= (sphere.radius * sphere.radius);
}

Indexed_Triangle_Mesh_Object::Indexed_Triangle_Mesh_Object(const std::vector<Triangle> &triangles,
                                                           const glm::mat4 &model_matrix)
    : m_model_matrix(model_matrix) {
  if (triangles.empty()) {
    throw GeoBox_Error("Empty mesh");
  }

  m_normal_matrix = glm::transpose(glm::inverse(model_matrix));

  size_t num_vertices = triangles.size() * 3;

  std::vector<AABB> vertices_as_bounding_boxes;
  vertices_as_bounding_boxes.reserve(num_vertices);
  for (const Triangle &triangle : triangles) {
    for (const glm::vec3 &v : triangle.vertices) {
      AABB aabb;
      aabb.min = v;
      aabb.max = v;
      vertices_as_bounding_boxes.push_back(aabb);
    }
  }

  std::shared_ptr<BVH> bvh;
  try {
    bvh = std::make_shared<BVH>(vertices_as_bounding_boxes);
  } catch (const GeoBox_Error &) {
    std::cerr << "Failed to build vertices BVH" << std::endl;
    throw; // rethrows original error
  }
  assert(bvh->count_primitives() == num_vertices);
  std::cout << "Num nodes = " << bvh->count_nodes() << std::endl;
  std::cout << "Max node size = " << bvh->calc_max_leaf_size() << std::endl;

  std::vector<unsigned int> indices(num_vertices);
  std::vector<bool> is_remapped_vec(num_vertices, false);
  std::vector<glm::vec3> unique_vertices;
  unique_vertices.reserve(num_vertices);

  auto get_ith_vertex = [&triangles](unsigned int i) -> const glm::vec3 & {
    auto div = std::ldiv(i, 3);
    size_t triangle_index = div.quot;
    assert(triangle_index >= 0 && triangle_index < triangles.size());
    size_t vertex_index_in_triangle = div.rem;
    assert(vertex_index_in_triangle >= 0 && vertex_index_in_triangle <= 2);
    return triangles[triangle_index].vertices[vertex_index_in_triangle];
  };

  unsigned int final_unique_vertex_index = 0;
  for (unsigned int original_unique_vertex_index = 0; original_unique_vertex_index < num_vertices;
       original_unique_vertex_index++) {
    // Skip already remapped vertex
    if (is_remapped_vec[original_unique_vertex_index])
      continue;
    const glm::vec3 &unique_vertex = get_ith_vertex(original_unique_vertex_index);
    constexpr float range = 0.0001f;
    Sphere sphere{.center = unique_vertex, .radius = range};

    auto duplicate_vertex_callback = [final_unique_vertex_index, &is_remapped_vec,
                                      &indices](unsigned int duplicate_vertex_index) {
      if (!is_remapped_vec[duplicate_vertex_index]) {
        indices[duplicate_vertex_index] = final_unique_vertex_index;
        is_remapped_vec[duplicate_vertex_index] = true;
      }
    };
    auto aabb_filter = [&const_sphere = std::as_const(sphere)](const AABB &aabb) {
      return sphere_aabb_intersection(const_sphere, aabb);
    };
    auto primitive_filter = [&unique_vertex, &get_ith_vertex](unsigned int potential_duplicate_vertex_index) {
      return glm::distance2(unique_vertex, get_ith_vertex(potential_duplicate_vertex_index)) <= (range * range);
    };
    bvh->foreach_primitive(duplicate_vertex_callback, aabb_filter, primitive_filter);

    final_unique_vertex_index++;
    unique_vertices.push_back(unique_vertex);
  }
  unique_vertices.shrink_to_fit();

  std::cout << "Num unique vertices = " << unique_vertices.size() << std::endl;

  // Pre-calculate number of triangles per vertex (can be used later for weighting normals)
  std::vector<float> num_triangles_per_vertex(unique_vertices.size(), 0.0f);
  for (unsigned int vi : indices) {
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
    throw Overflow_Check_Error("Aborting GPU mesh creation, too many vertices, TODO: support larger meshes");
  }
  auto unique_vertices_buffer_size = static_cast<unsigned int>(unique_vertices.size() * sizeof(glm::vec3));

  if (indices.size() > (std::numeric_limits<int>::max() / sizeof(unsigned int))) {
    throw Overflow_Check_Error("Aborting GPU mesh creation, too many indices, TODO: support larger meshes");
  }

  // Set CPU mesh
  m_vertices = unique_vertices;
  m_indices = indices;
  m_vertex_normals = vertex_normals;

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

  m_num_indices = static_cast<int>(indices.size());
  int indices_buffer_size = m_num_indices * static_cast<int>(sizeof(unsigned int));
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_size, indices.data(), GL_STATIC_DRAW);

  // Vertex normals buffer has same size as vertex positions buffer if calculated properly
  unsigned int vertex_normals_buffer_size = unique_vertices_buffer_size;
  unsigned int vertex_normals_buffer_object;
  glGenBuffers(1, &vertex_normals_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_normals_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, vertex_normals_buffer_size, m_vertex_normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(1);

  m_VAO = VAO;
  m_vertex_positions_buffer_object = vertex_positions_buffer_object;
  m_vertex_normals_buffer_object = vertex_normals_buffer_object;
  m_EBO = EBO;

  // Build BVH for triangles
  std::vector<AABB> triangle_bounding_boxes;
  for (unsigned int i = 0; i < indices.size(); i += 3) {
    const glm::vec3 &a = unique_vertices[indices[i + 0]];
    const glm::vec3 &b = unique_vertices[indices[i + 1]];
    const glm::vec3 &c = unique_vertices[indices[i + 2]];
    AABB aabb;
    aabb.min = glm::min(a, glm::min(b, c));
    aabb.max = glm::max(a, glm::max(b, c));
    triangle_bounding_boxes.push_back(aabb);
  }

  try {
    m_triangles_bvh = std::make_shared<BVH>(triangle_bounding_boxes);
  } catch (const GeoBox_Error &) {
    std::cerr << "Failed to build triangles BVH" << std::endl;
    throw; // rethrows original error
  }

  // Compute and store triangle areas
  m_triangle_areas.reserve(triangles.size());
  for (unsigned int i = 0; i < indices.size(); i += 3) {
    const glm::vec3 &a = unique_vertices[indices[i + 0]];
    const glm::vec3 &b = unique_vertices[indices[i + 1]];
    const glm::vec3 &c = unique_vertices[indices[i + 2]];
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    float area = glm::length(glm::cross(ab, ac)) * 0.5f;
    m_triangle_areas.push_back(area);
  }
}

void Indexed_Triangle_Mesh_Object::draw() const {
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, nullptr);
}

Indexed_Triangle_Mesh_Object::~Indexed_Triangle_Mesh_Object() {
  glDeleteVertexArrays(1, &m_VAO);
  glDeleteBuffers(1, &m_vertex_positions_buffer_object);
  glDeleteBuffers(1, &m_vertex_normals_buffer_object);
  glDeleteBuffers(1, &m_EBO);
}

#include <fstream>  // for std::ifstream
#include <iostream> // for std::cerr, std::endl, etc...
#include <optional>
#include <string>
#include <vector>

#include "triangle.hpp"

constexpr size_t BINARY_STL_HEADER_SIZE = 80;

static size_t calc_expected_binary_stl_mesh_file_size(uint32_t num_triangles) {
  return BINARY_STL_HEADER_SIZE + sizeof(uint32_t) + num_triangles * (sizeof(float[4][3]) + sizeof(uint16_t));
}

static std::vector<Triangle> read_stl_mesh_file_binary(std::ifstream &ifs, uint32_t num_triangles) {
  std::vector<Triangle> triangles;
  for (uint32_t i = 0; i < num_triangles; i++) {
    ifs.seekg(sizeof(glm::vec3), std::ifstream::cur); // Skip normals
    // Yes we can do the reading differently but this way is nice and explicit for now
    for (glm::vec3 &vertex : triangles.emplace_back().vertices) {
      ifs.read((char *)&vertex, sizeof(glm::vec3));
    }
    // Skip "attribute byte count"
    ifs.seekg(sizeof(uint16_t), std::ifstream::cur);
  }
  return triangles;
}

static std::vector<Triangle> read_stl_mesh_file_ascii(std::ifstream &ifs) {
  std::vector<Triangle> triangles;
  std::string token;
  while (ifs.good()) {
    ifs >> token;
    if (token == "facet") {
      ifs >> token;                   // expecting "normal"
      ifs >> token >> token >> token; // Skip x, y and z of normal vector
      ifs >> token;                   // expecting "outer"
      ifs >> token;                   // expecting "loop"
      for (glm::vec3 &vertex : triangles.emplace_back().vertices) {
        ifs >> token; // expecting "vertex"
        ifs >> vertex.x >> vertex.y >> vertex.z;
      }
      ifs >> token; // expecting "endloop"
      ifs >> token; // expecting "endfacet"
    }
  }
  return triangles;
}

std::optional<std::vector<Triangle>> read_stl_mesh_file(const std::string &file_path) {
  std::ifstream ifs(file_path, std::ifstream::ate | std::ifstream::binary);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open file: " << file_path << std::endl;
    return {};
  }

  auto file_end = ifs.tellg(); // file is already at end if it was opened in "ate" mode
  ifs.seekg(0, std::ifstream::beg);
  auto file_beg = ifs.tellg();
  auto file_size = file_end - file_beg;

  if (file_size == 0) {
    std::cerr << "Empty file: " << file_path << std::endl;
    return {};
  }
  // Assume file is binary at first and skip binary header
  ifs.seekg(BINARY_STL_HEADER_SIZE, std::ifstream::beg);

  uint32_t num_triangles = 0;
  ifs.read((char *)&num_triangles, sizeof(uint32_t));

  if (file_size == calc_expected_binary_stl_mesh_file_size(num_triangles)) {
    return read_stl_mesh_file_binary(ifs, num_triangles);
  } else {
    ifs.seekg(0, std::ifstream::beg);
    return read_stl_mesh_file_ascii(ifs);
  }
}

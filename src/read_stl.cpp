#include <cstddef>  // for std::size_t
#include <cstdint>  // for uint16_t, uint32_t, etc...
#include <fstream>  // for std::ifstream
#include <iostream> // for std::cerr, std::endl, etc...
#include <optional>
#include <string>
#include <vector>

#include <glm/vec3.hpp>

constexpr size_t BINARY_STL_HEADER_SIZE = 80;

[[maybe_unused]] static size_t calc_file_size(std::ifstream &ifs) {
  auto original_pos = ifs.tellg();
  ifs.seekg(0, std::ifstream::end);
  auto end = ifs.tellg();
  ifs.seekg(0, std::ifstream::beg);
  size_t file_size = end - ifs.tellg();
  ifs.seekg(original_pos, std::ifstream::beg);
  return file_size;
}

static size_t calc_expected_binary_stl_mesh_file_size(uint32_t num_triangles) {
  return BINARY_STL_HEADER_SIZE + sizeof(uint32_t) + num_triangles * (sizeof(float[4][3]) + sizeof(uint16_t));
}

static std::vector<glm::vec3> read_stl_mesh_file_binary(std::ifstream &ifs, uint32_t num_triangles) {
  std::vector<glm::vec3> vertices;
  vertices.reserve(num_triangles * 3);
  for (uint32_t i = 0; i < num_triangles; i++) {
    ifs.seekg(sizeof(glm::vec3), std::ifstream::cur); // Skip normals
    for (int j = 0; j < 3; j++) {
      ifs.read((char *)&vertices.emplace_back(), sizeof(glm::vec3));
    }
    // Skip "attribute byte count"
    ifs.seekg(sizeof(uint16_t), std::ifstream::cur);
  }
  return vertices;
}

static std::vector<glm::vec3> read_stl_mesh_file_ascii(std::ifstream &ifs) {
  std::vector<glm::vec3> vertices;
  std::string token;
  while (ifs.good()) {
    ifs >> token;
    if (token == "facet") {
      ifs >> token;                   // expecting "normal"
      ifs >> token >> token >> token; // Skip normal
      ifs >> token;                   // expecting "outer"
      ifs >> token;                   // expecting "loop"
      for (int i = 0; i < 3; i++) {
        ifs >> token; // expecting "vertex"
        glm::vec3 &vertex = vertices.emplace_back();
        ifs >> vertex.x >> vertex.y >> vertex.z;
      }
      ifs >> token; // expecting "endloop"
      ifs >> token; // expecting "endfacet"
    }
  }
  return vertices;
}

std::optional<std::vector<glm::vec3>> read_stl_mesh_file(const std::string &file_path) {
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

#pragma once

#include <optional>
#include <vector>
#include <string>

#include <glm/vec3.hpp>

std::optional<std::vector<glm::vec3>> read_stl_mesh_file(const std::string &file_path);
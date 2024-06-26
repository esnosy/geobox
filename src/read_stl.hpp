#pragma once

#include <optional>
#include <string>
#include <vector>

#include "primitives.hpp"

[[nodiscard]] std::optional<std::vector<Triangle>> read_stl_mesh_file(const std::string &file_path);

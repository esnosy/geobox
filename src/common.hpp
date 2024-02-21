#pragma once

#include <glm/vec3.hpp>

[[nodiscard]] bool is_close(float a, float b);
[[nodiscard]] bool is_close(const glm::vec3 &a, const glm::vec3 &b);

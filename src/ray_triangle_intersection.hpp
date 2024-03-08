#pragma once

#include <optional>

#include "ray.hpp"
#include "triangle.hpp"

[[nodiscard]] std::optional<float> ray_intersects_triangle_non_coplanar(const Ray &ray, const Triangle &triangle);

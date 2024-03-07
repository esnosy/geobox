#pragma once

#include "ray.hpp"
#include "triangle.hpp"

[[nodiscard]] bool ray_intersects_triangle_non_coplanar(const Ray &ray, const Triangle &triangle);

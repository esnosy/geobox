#pragma once

#include <optional>

#include "aabb.hpp"
#include "ray.hpp"

[[nodiscard]] std::optional<float> ray_aabb_intersection(const Ray &ray, const AABB &aabb);

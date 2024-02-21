#pragma once

#include "aabb.hpp"
#include "ray.hpp"

[[nodiscard]] float ray_aabb_intersection(const Ray &ray, const AABB &aabb);

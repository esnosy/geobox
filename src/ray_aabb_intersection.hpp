#pragma once

#include "aabb.hpp"
#include "ray.hpp"

float ray_aabb_intersection(const Ray &ray, const AABB &aabb);
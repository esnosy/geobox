#pragma once

#include <optional>

#include "primitives.hpp"
#include "math.hpp"

std::optional<glm::vec3> intersect(const Tolerance_Context &tc, const Triangle &t, const Segment &s);

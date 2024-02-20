#include <algorithm> // for std::min and std::max
#include <cassert>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "aabb.hpp"
#include "common.hpp"
#include "ray.hpp"

static float min_component(const glm::vec3 &v) { return std::min(v.x, std::min(v.y, v.z)); }

static float max_component(const glm::vec3 &v) { return std::max(v.x, std::min(v.y, v.z)); }

static bool is_not_all_zeros(const glm::vec3 &v) { return glm::any(glm::greaterThan(glm::abs(v), glm::vec3(0))); }

float ray_aabb_intersection(const Ray &ray, const AABB &aabb) {
  assert(is_not_all_zeros(ray.direction));
  // ray-aabb intersection:
  // https://gist.github.com/bromanz/a267cdf12f6882a25180c3724d807835/4929f6d8c3b2ae1facd1d655c8d6453603c465ce
  // https://web.archive.org/web/20240108120351/https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
  glm::vec3 t_slab_min(0);
  glm::vec3 t_slab_max(std::numeric_limits<float>::infinity());
  for (int i = 0; i < 3; i++) {
    if (!is_close(ray.direction[i], 0)) {
      // the use of std::tie https://stackoverflow.com/a/74057204/8094047
      std::tie(t_slab_min[i], t_slab_max[i]) = std::minmax((aabb.min[i] - ray.origin[i]) / ray.direction[i],
                                                           (aabb.max[i] - ray.origin[i]) / ray.direction[i]);
    }
  }
  float t_min = std::max(max_component(t_slab_min), 0.0f);
  float t_max = min_component(t_slab_max);
  return std::min(t_min, t_max);
}

#ifdef TEST_RAY_AABB_INTERSECTION
int main() {
  AABB aabb{.min = glm::vec3(-1), .max = glm::vec3(1)};

  struct Test_Case {
    Ray ray;
    AABB aabb;
    bool does_intersect;
  };

  std::vector<Test_Case> cases = {
      {{glm::vec3(2), glm::vec3(-1)}, aabb, true},
      {{glm::vec3(2), glm::vec3(1)}, aabb, false},
      {{glm::vec3(-2), glm::vec3(-1)}, aabb, false},
      {{glm::vec3(-2), glm::vec3(1)}, aabb, true},
      // Some edge cases,
      {{aabb.min - glm::vec3(0), glm::vec3(-1)}, aabb, true},
      {{aabb.min - glm::vec3(0), glm::vec3(1)}, aabb, true},
      {{aabb.min - glm::vec3(0.0001f), glm::vec3(-1)}, aabb, false},
      {{aabb.min - glm::vec3(0.0001f), glm::vec3(1)}, aabb, true},
      {{aabb.min + glm::vec3(0.0001f), glm::vec3(1)}, aabb, true},
      {{aabb.min + glm::vec3(0.0001f), glm::vec3(-1)}, aabb, true},
      {{aabb.min - glm::vec3(0, 0, 3), glm::vec3(0, 0, 1)}, aabb, true},
      {{glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)}, aabb, true},
      {{glm::vec3(0, 0, 1), glm::vec3(1, 0, 0)}, aabb, true},
      {{glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)}, aabb, true},
      // Other case
      {{glm::vec3(0), glm::vec3(0, 0, 1)}, aabb, true},
      {{glm::vec3(0), glm::vec3(0, 1, 0)}, aabb, true},
      {{glm::vec3(0), glm::vec3(1, 0, 0)}, aabb, true},
      {{glm::vec3(0), glm::vec3(-1, 0, 0)}, aabb, true},
      {{glm::vec3(0), glm::vec3(0, -1, 0)}, aabb, true},
      {{glm::vec3(0), glm::vec3(0, 0, -1)}, aabb, true},
      {{glm::vec3(0), glm::vec3(1, 1, 1)}, aabb, true},
      {{glm::vec3(0, 0, 2), glm::vec3(0, 0, 1)}, aabb, false},
      {{glm::vec3(0, 0, -2), glm::vec3(0, 0, -1)}, aabb, false},
      {{glm::vec3(2, -2, -2), glm::vec3(2, -2, -2)}, aabb, false},
      {{glm::vec3(2, -2, -2), -glm::vec3(2, -2, -2)}, aabb, true},
  };

  int i = 0;
  float t;
  for (const Test_Case &c : cases) {
    std::cout << "Test Case: " << i << std::endl;
    t = ray_aabb_intersection(c.ray, c.aabb);
    if ((t >= 0) != c.does_intersect) {
      std::abort();
    }
    i++;
  }

  return 0;
}
#endif

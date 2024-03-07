#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>

#include <glm/glm.hpp>

#include "ray_triangle_intersection.hpp"

bool ray_intersects_triangle_non_coplanar(const Ray &ray, const Triangle &triangle) {
  glm::vec3 ab = triangle.vertices[1] - triangle.vertices[0];
  glm::vec3 ac = triangle.vertices[2] - triangle.vertices[0];
  // First column of cofactor matrix
  float c11 = ab.y * ac.z - ab.z * ac.y;
  float c21 = ab.z * ac.x - ab.x * ac.z;
  float c31 = ab.x * ac.y - ab.y * ac.x;
  float det = ray.direction.x * c11 + ray.direction.y * c21 + ray.direction.z * c31;
  constexpr float epsilon = 1e-5f;
  if (std::abs(det) < epsilon)
    return false;
  // Second column of cofactor matrix
  float c12 = ray.direction.y * ac.z - ray.direction.z * ac.y;
  float c22 = ray.direction.z * ac.x - ray.direction.x * ac.z;
  float c32 = ray.direction.x * ac.y - ray.direction.y * ac.x;

  // Third column of cofactor matrix
  float c13 = ray.direction.z * ab.y - ray.direction.y * ab.z;
  float c23 = ray.direction.x * ab.z - ray.direction.z * ab.x;
  float c33 = ray.direction.y * ab.x - ray.direction.x * ab.y;

  // Constant column vector
  glm::vec3 c = triangle.vertices[0] - ray.origin;
  float t = (c11 * c.x + c21 * c.y + c31 * c.z) / det;
  if (t < -epsilon)
    return false;
  float u = (c12 * c.x + c22 * c.y + c32 * c.z) / det;
  if (u < -epsilon)
    return false;
  float v = (c13 * c.x + c23 * c.y + c33 * c.z) / det;
  if (v < -epsilon)
    return false;
  if ((u + v) > (1.0f + epsilon))
    return false;
  return true;
}

#ifdef TEST_RAY_TRIANGLE_INTERSECTION
int main() {
  std::array<unsigned int, 3> seeds = {1, 2, 3};
  std::mt19937 e1(seeds[0]);
  std::mt19937 e2(seeds[1]);
  std::mt19937 e3(seeds[2]);
  std::uniform_real_distribution dist(-10.0f, 10.0f);

  auto get_random_vec3 = [&]() { return glm::vec3{dist(e1), dist(e2), dist(e3)}; };

  for (size_t i = 0; i < 10; i++) {
    std::cout << "Test Case: " << i << std::endl;
    Triangle triangle{{get_random_vec3(), get_random_vec3(), get_random_vec3()}};

    glm::vec3 normal = glm::normalize(
        glm::cross(triangle.vertices[1] - triangle.vertices[0], triangle.vertices[2] - triangle.vertices[0]));
    glm::vec3 center = (triangle.vertices[0] + triangle.vertices[1] + triangle.vertices[2]) / 3.0f;
    glm::vec3 ray_origin = center + normal; // Good origin for non-coplanar ray

    // Rays through vertices should hit
    for (const glm::vec3 &v : triangle.vertices) {
      Ray ray{ray_origin, glm::normalize(v - ray_origin)};
      if (!ray_intersects_triangle_non_coplanar(ray, triangle)) {
        std::abort();
      }
    }

    // Ray through center of triangle should hit
    Ray ray{ray_origin, -normal};
    if (!ray_intersects_triangle_non_coplanar(ray, triangle))
      std::abort();

    // Ray through scaled up triangle should miss original triangle
    Triangle scaled_triangle = triangle;
    for (glm::vec3 &v : scaled_triangle.vertices) {
      v = (v - center) * 2.0f + center;
      Ray ray{ray_origin, glm::normalize(v - ray_origin)};
      if (ray_intersects_triangle_non_coplanar(ray, triangle)) {
        std::abort();
      }
    }
  }
}
#endif

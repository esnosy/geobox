#pragma once

#include <optional>
#include <vector>

#include <glm/vec3.hpp>

class BVH {
public:
  struct Node {
    struct AABB {
      glm::vec3 min, max;
    };
    AABB aabb;
    unsigned int *first, *last;
    Node *left, *right;
  };

private:
  unsigned int *m_primitive_indices = nullptr;
  Node *m_current_free_node = nullptr;
  Node *m_pre_allocated_nodes = nullptr;
  Node *m_root = nullptr;
  Node *new_node();

public:
  BVH() = delete;
  // Memory is freed in destructor, avoid double free by disabling copy
  // also known as the "Rule of three"
  BVH(BVH const &) = delete;
  BVH &operator=(BVH const &) = delete;

  explicit BVH(std::vector<glm::vec3> const &points);
  ~BVH();
  bool is_empty() const;
  size_t count_nodes() const;
};

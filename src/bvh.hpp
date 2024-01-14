#pragma once

#include <cassert>
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

    [[nodiscard]] bool is_leaf() const { return (left == nullptr) && (right == nullptr); }

    [[nodiscard]] unsigned int num_primitives() const {
      assert(last >= first);
      return last - first + 1;
    }
  };

private:
  unsigned int *m_primitive_indices = nullptr;
  Node *m_current_free_node = nullptr;
  Node *m_pre_allocated_nodes = nullptr;
  Node *m_root = nullptr;
  Node *new_node();
  void foreach_node(std::function<void(const Node *)> const &callback,
                    std::function<bool(Node::AABB const &aabb)> const &aabb_filter) const;
  void foreach_node_leaf(std::function<void(const Node *)> const &callback,
                         std::function<bool(Node::AABB const &aabb)> const &aabb_filter) const;

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
  [[nodiscard]] unsigned int calc_max_leaf_size() const;
  [[nodiscard]] unsigned int count_primitives() const;

  void foreach_primitive(std::function<void(unsigned int)> const &callback,
                         std::function<bool(Node::AABB const &aabb)> const &aabb_filter,
                         std::function<bool(unsigned int)> const &primitive_filter) const;
};

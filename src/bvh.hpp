#pragma once

#include <cassert>
#include <functional>
#include <optional>
#include <vector>

#include <glm/vec3.hpp>

class BVH {
public:
  struct Node {
    struct AABB {
      glm::vec3 min;
      glm::vec3 max;
    };

    AABB aabb;
    unsigned int *first;
    unsigned int *last;
    Node *left;
    Node *right;

    [[nodiscard]] bool is_leaf() const { return (left == nullptr) && (right == nullptr); }

    [[nodiscard]] size_t num_primitives() const {
      assert(last >= first);
      return last - first + 1;
    }
  };

private:
  unsigned int *m_primitive_indices = nullptr;
  Node *m_current_free_node = nullptr;
  Node *m_pre_allocated_nodes = nullptr;
  Node *m_root = nullptr;
  bool m_did_build_fail = false;
  Node *new_node();
  template <typename Callback_Type, typename AABB_Filter_Type>
  void foreach_node(Callback_Type callback, AABB_Filter_Type aabb_filter) const;
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
  [[nodiscard]] size_t count_nodes() const;
  [[nodiscard]] size_t calc_max_leaf_size() const;
  [[nodiscard]] size_t count_primitives() const;

  [[nodiscard]] bool did_build_fail() const;

  void foreach_primitive(std::function<void(unsigned int)> const &callback,
                         std::function<bool(Node::AABB const &aabb)> const &aabb_filter,
                         std::function<bool(unsigned int)> const &primitive_filter) const;
};

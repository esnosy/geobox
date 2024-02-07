#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "aabb.hpp"

class BVH {
public:
  struct Node {

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
  Node *new_node();
  template <typename Callback_Type, typename AABB_Filter_Type>
  void foreach_node(Callback_Type callback, AABB_Filter_Type aabb_filter) const;
  void foreach_leaf_node(const std::function<void(const Node *)> &callback,
                         const std::function<bool(AABB const &aabb)> &aabb_filter) const;

  // Private default constructor to only allow creating objects through public API
  BVH() = default;

public:
  // Memory is freed in destructor,
  // avoid double free by disabling copy constructor and copy assignment operator,
  // also known as the "Rule of three"
  BVH(const BVH &) = delete;
  BVH &operator=(const BVH &) = delete;

  // We use shared pointer to still allow using the object easily in multiple places and still avoid double free and
  // prevent use after free
  [[nodiscard]] static std::shared_ptr<BVH> from_bounding_boxes(const std::vector<AABB> &bounding_boxes);
  ~BVH();
  [[nodiscard]] size_t count_nodes() const;
  [[nodiscard]] size_t calc_max_leaf_size() const;
  [[nodiscard]] size_t count_primitives() const;

  void foreach_primitive(const std::function<void(unsigned int)> &callback,
                         const std::function<bool(AABB const &aabb)> &aabb_filter,
                         const std::function<bool(unsigned int)> &primitive_filter) const;
};

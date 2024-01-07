#include <algorithm> // for std::partition
#include <iostream>
#include <stack>

#include <glm/common.hpp>

#include "bvh.hpp"

BVH::Node *BVH::new_node(Node &&init) {
  *m_current_free_node = init;
  return m_current_free_node++;
}

static BVH::Node::AABB calc_aabb_indirect(std::vector<glm::vec3> const &points, unsigned int const *first,
                                          unsigned int const *last) {
  if (points.empty()) {
    return {};
  }
  BVH::Node::AABB aabb{.min = points[*first], .max = points[*first]};
  for (unsigned int const *i = first; i <= last; i++) {
    aabb.min = glm::min(aabb.min, points[*i]);
  }
  // Separate loop, so even without compiler optimization, similar instructions are in a loop
  for (unsigned int const *i = first; i <= last; i++) {
    aabb.max = glm::max(aabb.max, points[*i]);
  }
  return aabb;
}

BVH::BVH(std::vector<glm::vec3> const &points) {
  if (points.empty()) {
    std::cerr << "Empty points, aborting creation of BVH..." << std::endl;
    return;
  }

  // Pre-allocate nodes
  m_pre_allocated_nodes = (Node *)malloc(sizeof(Node) * (2 * points.size() - 1));
  m_current_free_node = m_pre_allocated_nodes;

  // Build initial indices array
  m_primitive_indices = (unsigned int *)malloc(sizeof(unsigned int) * points.size());
  for (unsigned int i = 0; i < points.size(); i++) {
    m_primitive_indices[i] = i;
  }

  // Create root
  m_root = new_node({
      .first = m_primitive_indices,
      .last = m_primitive_indices + points.size() - 1,
      .left = nullptr,
      .right = nullptr,
  });

  // Build tree
  std::stack<Node *> stack;
  stack.push(m_root);
  while (!stack.empty()) {
    Node *node = stack.top();
    stack.pop();
    assert(node->first <= node->last);

    // Calculate AABB
    node->aabb = calc_aabb_indirect(points, node->first, node->last);

    // Skip splitting of nodes that contain a single primitive
    if (node->first == node->last) {
      continue;
    }

    // Calculate variance
    glm::vec3 mean_of_squares(0), mean(0);
    float num_node_vertices = static_cast<float>(node->last - node->first + 1);
    for (unsigned int *i = node->first; i <= node->last; i++) {
      const glm::vec3 &v = points[*i];
      glm::vec3 tmp = v / num_node_vertices;
      mean += tmp;
      mean_of_squares += v * tmp;
    }
    glm::vec3 variance = mean_of_squares - mean * mean;

    // Determine the axis of greatest variance and split position
    std::uint8_t axis = 0;
    if (variance[1] > variance[0]) {
      axis = 1;
    }
    if (variance[2] > variance[axis]) {
      axis = 2;
    }
    float split_pos = mean[axis];

    // Partition vertices
    // note that std::partition input range is not inclusive,
    // so if we need to include the "last" value in partitioning, we pass last + 1 to std::partition as the "last"
    // parameter: https://en.cppreference.com/mwiki/index.php?title=cpp/algorithm/partition&oldid=150246
    unsigned int *second_group_first =
        std::partition(node->first, node->last + 1,
                       [&points, axis, split_pos](unsigned int i) { return points[i][axis] < split_pos; });

    // Abort current node if partitioning fails
    if (second_group_first == node->first || second_group_first == (node->last + 1)) {
      continue;
    }

    node->left = new_node({
        .first = node->first,
        .last = second_group_first - 1,
        .left = nullptr,
        .right = nullptr,
    });
    node->right = new_node({
        .first = second_group_first,
        .last = node->last,
        .left = nullptr,
        .right = nullptr,
    });
    stack.push(node->left);
    stack.push(node->right);
  }
}

BVH::~BVH() {
  std::stack<Node *> stack;
  stack.push(m_root);
  int num_nodes = 1;
  while (!stack.empty()) {
    const Node *node = stack.top();
    stack.pop();
    if (node->left) {
      num_nodes++;
      stack.push(node->left);
    }
    if (node->right) {
      num_nodes++;
      stack.push(node->right);
    }
  }
  free(m_primitive_indices);
  free(m_pre_allocated_nodes);
}

bool BVH::is_empty() const { return m_root == nullptr; }
size_t BVH::count_nodes() const {
  size_t num_nodes = 0;
  std::stack<Node *> stack;
  stack.push(m_root);
  while (!stack.empty()) {
    const Node *node = stack.top();
    stack.pop();
    num_nodes++;
    if (node->left) {
      stack.push(node->left);
    }
    if (node->right) {
      stack.push(node->right);
    }
  }
  return num_nodes;
}

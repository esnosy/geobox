#include <algorithm> // for std::partition, std::min, std::max, std::minmax, std::min_element and std::max_element
#include <cstdlib>   // for std::malloc, std::free, std::abort and std::abs
#include <functional>
#include <iostream>
#include <limits> // for std::numeric_limits
#include <memory> // for std::shared_ptr
#include <stack>
#include <tuple>   // for std::tie
#include <utility> // for std::as_const
#include <vector>

#include <glm/common.hpp> // for glm::min and glm::max
#include <glm/gtx/norm.hpp>

#include "bvh.hpp"
#include "common.hpp"

struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;
};

float min_component(const glm::vec3 &v) { return std::min(v.x, std::min(v.y, v.z)); }

float max_component(const glm::vec3 &v) { return std::max(v.x, std::min(v.y, v.z)); }

bool is_not_all_zeros(const glm::vec3 &v) { return glm::any(glm::greaterThan(glm::abs(v), glm::vec3(0))); }

static float ray_aabb_intersection(const Ray &ray, BVH::Node::AABB const &aabb) {
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
  BVH::Node::AABB aabb{.min = glm::vec3(-1), .max = glm::vec3(1)};

  struct Test_Case {
    Ray ray;
    BVH::Node::AABB aabb;
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

BVH::Node *BVH::new_node() { return m_current_free_node++; }

static BVH::Node::AABB calc_aabb_indirect(const std::vector<BVH::Node::AABB> &bounding_boxes, const unsigned int *first,
                                          const unsigned int *last) {
  BVH::Node::AABB aabb = bounding_boxes[*first];
  for (const unsigned int *i = first; i <= last; i++) {
    aabb.min = glm::min(aabb.min, bounding_boxes[*i].min);
  }
  // Separate loop, so even without compiler optimization, similar instructions are in a loop
  for (const unsigned int *i = first; i <= last; i++) {
    aabb.max = glm::max(aabb.max, bounding_boxes[*i].max);
  }
  return aabb;
}

std::shared_ptr<BVH> BVH::from_bounding_boxes(const std::vector<Node::AABB> &bounding_boxes) {
  size_t num_primitives = bounding_boxes.size();
  if (num_primitives == 0) {
    std::cerr << "Zero number of primitives, aborting creation of BVH..." << std::endl;
    return {};
  }

  // Calculate bounding box centers
  std::vector<glm::vec3> bounding_box_centers;
  bounding_box_centers.reserve(bounding_boxes.size());
  for (const Node::AABB &aabb : bounding_boxes) {
    // We multiply by 0.5f first (as opposed to multiplying after adding min and max) to reduce the values of min and
    // max to support larger values of min and max
    bounding_box_centers.emplace_back(aabb.min * 0.5f + aabb.max * 0.5f);
  }

  // We cannot use std::make_shared directly if constructor is private
  auto result = std::shared_ptr<BVH>(new BVH());

  // Pre-allocate nodes
  result->m_pre_allocated_nodes = (Node *)malloc(sizeof(Node) * (2 * num_primitives - 1));
  result->m_current_free_node = result->m_pre_allocated_nodes;

  // Build initial indices array
  result->m_primitive_indices = (unsigned int *)malloc(sizeof(unsigned int) * num_primitives);
  for (unsigned int i = 0; i < num_primitives; i++) {
    result->m_primitive_indices[i] = i;
  }

  // Create root
  result->m_root = result->new_node();
  *(result->m_root) = {
      .first = result->m_primitive_indices,
      .last = result->m_primitive_indices + num_primitives - 1,
      .left = nullptr,
      .right = nullptr,
  };

  // Build tree
  std::stack<Node *> stack;
  stack.push(result->m_root);
  while (!stack.empty()) {
    Node *node = stack.top();
    stack.pop();
    assert(node->first <= node->last);

    // Calculate AABB
    node->aabb = calc_aabb_indirect(bounding_boxes, node->first, node->last);

    // Skip splitting of nodes that contain a single primitive
    if (node->first == node->last) {
      continue;
    }

    // Calculate variance
    auto num_primitives_as_float = static_cast<float>(node->num_primitives());
    if (num_primitives_as_float > std::numeric_limits<float>::max()) {
      std::cerr << "Too many primitives for variance calculation, aborting BVH build" << std::endl;
      return {};
    }
    glm::vec3 mean_of_squares(0);
    glm::vec3 mean(0);
    for (const unsigned int *i = node->first; i <= node->last; i++) {
      const glm::vec3 &v = bounding_box_centers[*i];
      glm::vec3 tmp = v / num_primitives_as_float;
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

    // Partition primitives
    // note that std::partition input range is not inclusive,
    // so if we need to include the "last" value in partitioning, we pass last + 1 to std::partition as the "last"
    // parameter: https://en.cppreference.com/mwiki/index.php?title=cpp/algorithm/partition&oldid=150246
    auto *second_group_first =
        std::partition(node->first, node->last + 1,
                       [&bounding_box_centers_const = std::as_const(bounding_box_centers), axis,
                        split_pos](unsigned int i) { return bounding_box_centers_const[i][axis] < split_pos; });

    // Abort current node if partitioning fails
    if (second_group_first == node->first || second_group_first == (node->last + 1)) {
      continue;
    }

    node->left = result->new_node();
    *(node->left) = {
        .first = node->first,
        .last = second_group_first - 1,
        .left = nullptr,
        .right = nullptr,
    };
    node->right = result->new_node();
    *(node->right) = {
        .first = second_group_first,
        .last = node->last,
        .left = nullptr,
        .right = nullptr,
    };
    stack.push(node->left);
    stack.push(node->right);
  }

  return result;
}

BVH::~BVH() {
  free(m_primitive_indices);
  free(m_pre_allocated_nodes);
}

size_t BVH::count_nodes() const {
  size_t num_nodes = 0;
  foreach_node([&num_nodes](const Node *) { num_nodes++; }, [](Node::AABB const &) { return true; });
  return num_nodes;
}

size_t BVH::calc_max_leaf_size() const {
  size_t max_node_size = 0;
  foreach_leaf_node(
      [&max_node_size](const Node *node) { max_node_size = std::max(max_node_size, node->num_primitives()); },
      [](Node::AABB const &) { return true; });
  return max_node_size;
}

size_t BVH::count_primitives() const {
  size_t num_primitives = 0;
  foreach_leaf_node([&num_primitives](const Node *node) { num_primitives += node->num_primitives(); },
                    [](Node::AABB const &) { return true; });
  return num_primitives;
}

template <typename Callback_Type, typename AABB_Filter_Type>
void BVH::foreach_node(Callback_Type callback, AABB_Filter_Type aabb_filter) const {
  std::stack<const Node *> stack;
  stack.push(m_root);
  while (!stack.empty()) {
    const Node *node = stack.top();
    stack.pop();

    if (!aabb_filter(node->aabb))
      continue;

    callback(node);
    if (!node->is_leaf()) {
      assert(node->left && node->right);
      stack.push(node->left);
      stack.push(node->right);
    }
  }
}

void BVH::foreach_leaf_node(const std::function<void(const Node *)> &callback,
                            const std::function<bool(BVH::Node::AABB const &aabb)> &aabb_filter) const {
  foreach_node(
      [&callback](const Node *node) {
        if (node->is_leaf())
          callback(node);
      },
      aabb_filter);
}

void BVH::foreach_primitive(const std::function<void(unsigned int)> &callback,
                            const std::function<bool(const BVH::Node::AABB &)> &aabb_filter,
                            const std::function<bool(unsigned int)> &primitive_filter) const {
  foreach_leaf_node(
      [&callback, &primitive_filter](const Node *node) {
        for (unsigned int const *i = node->first; i <= node->last; i++) {
          if (primitive_filter(*i))
            callback(*i);
        }
      },
      aabb_filter);
}

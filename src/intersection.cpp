#include <array>
#include <cassert>
#include <optional>
#include <utility>

#include <glm/glm.hpp>

#include "intersection.hpp"
#include "math.hpp"

static float sign_table[3][3] = {{1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}, {1.0f, -1.0f, 1.0f}};

bool is_close(const Tolerance_Context &tc, const glm::vec3 &a, const glm::vec3 &b) {
  return is_close(tc, a.x, b.x) && is_close(tc, a.y, b.y) && is_close(tc, a.z, b.z);
}

static float calc_cofactor(const std::array<glm::vec3, 3> &columns, int row, int column) {
  assert(row >= 0 && row <= 2);
  assert(column >= 0 && column <= 2);
  int minor_row = 0;
  int minor_column = 0;
  float minor[2][2];
  for (int i = 0; i < 3; i++) {
    if (i == row) continue;
    for (int j = 0; j < 3; j++) {
      if (j == column) continue;
      minor[minor_row][minor_column] = columns[j][i];
      minor_column++;
    }
    minor_column = 0;
    minor_row++;
  }
  return sign_table[row][column] * (minor[0][0] * minor[1][1] - minor[0][1] * minor[1][0]);
}

static glm::vec3 calc_cofactor_column(const std::array<glm::vec3, 3> &columns, int column) {
  return {
      calc_cofactor(columns, 0, column),
      calc_cofactor(columns, 1, column),
      calc_cofactor(columns, 2, column),
  };
}

// https://en.wikipedia.org/w/index.php?title=In-place_matrix_transposition&oldid=1217289860#Square_matrices
static std::array<glm::vec3, 3> transpose(const std::array<glm::vec3, 3> &matrix) {
  std::array<glm::vec3, 3> result = matrix;
  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      std::swap(result[i][j], result[j][i]);
    }
  }
  return result;
}

static std::optional<std::array<glm::vec3, 3>> invert(const Tolerance_Context &tc,
                                                      const std::array<glm::vec3, 3> &columns) {
  glm::vec3 cofactors_col_1 = calc_cofactor_column(columns, 0);
  float det = glm::dot(columns[0], cofactors_col_1);
  if (is_close(tc, det, 0.0f)) return std::nullopt;
  std::array<glm::vec3, 3> cofactor_cols = {cofactors_col_1, calc_cofactor_column(columns, 1),
                                            calc_cofactor_column(columns, 2)};
  std::array<glm::vec3, 3> adjugate_cols = transpose(cofactor_cols);
  adjugate_cols[0] /= det;
  adjugate_cols[1] /= det;
  adjugate_cols[2] /= det;
  return adjugate_cols;
}

static glm::vec3 transform(const std::array<glm::vec3, 3> &columns, const glm::vec3 &vector) {
  std::array<glm::vec3, 3> rows = transpose(columns);
  return glm::vec3{glm::dot(rows[0], vector), glm::dot(rows[1], vector), glm::dot(rows[2], vector)};
}

std::optional<glm::vec3> intersect(const Tolerance_Context &tc, const Triangle &t, const Segment &s) {
  std::array<glm::vec3, 3> coefficient_matrix_columns = {
      s.m_a - s.m_b,
      t.m_a - t.m_b,
      t.m_a - t.m_c,
  };
  auto inverse = invert(tc, coefficient_matrix_columns);
  if (!inverse.has_value()) return std::nullopt;
  glm::vec3 constant_vector = t.m_a - s.m_b;
  glm::vec3 tuv = transform(inverse.value(), constant_vector);
  return tuv.x * s.m_a + (1.0f - tuv.x) * s.m_b;
}

#ifdef GEOBOX_TEST_INTERSECTION
#include "testing.hpp"

int main() {
  Tolerance_Context tc(1e-9f, 1e-4f);

  // Test matrix transpose operation
  std::array<glm::vec3, 3> rows = {
      glm::vec3(1, 2, 3),
      glm::vec3(4, 5, 6),
      glm::vec3(7, 8, 9),
  };
  auto columns = transpose(rows);
  std::array<glm::vec3, 3> correct_columns = {
      glm::vec3(1, 4, 7),
      glm::vec3(2, 5, 8),
      glm::vec3(3, 6, 9),
  };
  for (int i = 0; i < 3; i++)
    runtime_assert(is_close(tc, columns[i], correct_columns[i]));

  // Test cofactors matrix calculation
  float correct_cofactors[3][3] = {
      {-3, 6, -3},
      {6, -12, 6},
      {-3, 6, -3},
  };
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      runtime_assert(is_close(tc, calc_cofactor(rows, i, j), correct_cofactors[i][j]));
    }
  }

  // Test matrix inverse calculation
  runtime_assert(!invert(tc, columns).has_value());

  std::array<glm::vec3, 3> invertable_columns = {
      glm::vec3(1, 2, 3),
      glm::vec3(3, 2, 1),
      glm::vec3(2, 1, 3),
  };
  auto inverse = invert(tc, invertable_columns);
  runtime_assert(inverse.has_value());
  std::array<glm::vec3, 3> correct_inverse = {
      glm::vec3(-5, 3, 4) / 12.0f,
      glm::vec3(7, 3, -8) / 12.0f,
      glm::vec3(1, -3, 4) / 12.0f,
  };
  for (int i = 0; i < 3; i++)
    runtime_assert(is_close(tc, inverse.value()[i], correct_inverse[i]));

  // Test vector transform
  std::array<glm::vec3, 3> transform_columns = {
      glm::vec3(1, 4, 7),
      glm::vec3(2, 5, 8),
      glm::vec3(3, 6, 9),
  };
  glm::vec3 vec{2, 1, 3};
  glm::vec3 transformed = transform(transform_columns, vec);
  glm::vec3 correct_transformed{13, 31, 49};
  runtime_assert(is_close(tc, transformed, correct_transformed));

  // Test triangle/segment intersection
  Segment s{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 100.0f}};
  Triangle tr{{0.0f, 0.0f, 0.00001f}, {0.0f, 100.0f, 0.00001f}, {100.0f, 0.0f, 0.00001f}};
  auto p = intersect(tc, tr, s);
  runtime_assert(p.has_value());
  runtime_assert(is_close(tc, p.value(), glm::vec3(0.0f, 0.0f, 0.00001f)));

  return 0;
}
#endif

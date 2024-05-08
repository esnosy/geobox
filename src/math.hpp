#pragma once

class Tolerance_Context {
private:
  float m_rel_tol, m_abs_tol;
  friend bool is_close(const Tolerance_Context &tc, float a, float b);

public:
  Tolerance_Context(float rel_tol, float abs_tol) : m_rel_tol(rel_tol), m_abs_tol(abs_tol) {}
  static Tolerance_Context get_default() { return Tolerance_Context(1e-9f, 1e-4f); }
};

// https://web.archive.org/web/20240419232447/https://docs.python.org/3/library/math.html#math.isclose
bool is_close(const Tolerance_Context &tc, float a, float b);

using TC = Tolerance_Context;

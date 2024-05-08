#pragma once

#include <cstdlib>

void runtime_assert(bool value) {
  if (!value) std::abort();
}

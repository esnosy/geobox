#pragma once

#include <functional>
#include <string>
#include <string_view>

class Shader {
private:
  unsigned int m_shader_program;
  int get_uniform_location(std::string_view uniform_name) const;

public:
  // Disable copy
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  Shader(const std::string &vertex_source_path, const std::string &fragment_source_path);
  ~Shader();
  void use() const;
  template <typename T> [[nodiscard]] std::function<void(const T &)> get_uniform_setter(std::string_view uniform_name);
};

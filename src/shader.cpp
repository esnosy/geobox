#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "geobox_exceptions.hpp"
#include "shader.hpp"

static std::optional<std::string> read_file_as_string(const std::string &file_path) {
  std::ifstream ifs(file_path, std::ifstream::binary);
  if (!ifs.is_open()) {
    throw GeoBox_Error("Failed to open file: " + file_path);
  }
  return std::string(std::istreambuf_iterator(ifs), {});
}

Shader::Shader(const std::string &vertex_shader_source_path, const std::string &fragment_shader_source_path) {
  std::optional<std::string> vertex_shader_source = read_file_as_string(vertex_shader_source_path);
  if (!vertex_shader_source.has_value()) {
    throw GeoBox_Error("Empty shader file: " + vertex_shader_source_path);
  }
  std::optional<std::string> fragment_shader_source = read_file_as_string(fragment_shader_source_path);
  if (!fragment_shader_source.has_value()) {
    throw GeoBox_Error("Empty shader file: " + fragment_shader_source_path);
  }

  std::vector<char *> vertex_shader_sources = {vertex_shader_source->data()};
  std::vector<char *> fragment_shader_sources = {fragment_shader_source->data()};
  int success;

  int max_info_log_length = 512;
  int actual_info_log_length = 0;
  std::vector<char> info_log(max_info_log_length);

  auto get_info_log_string = [&]() { return std::string(info_log.begin(), info_log.begin() + actual_info_log_length); };

  unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, (int)vertex_shader_sources.size(), vertex_shader_sources.data(), nullptr);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, max_info_log_length, &actual_info_log_length, info_log.data());
    throw GeoBox_Error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + get_info_log_string());
  }

  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, (int)fragment_shader_sources.size(), fragment_shader_sources.data(), nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, max_info_log_length, &actual_info_log_length, info_log.data());
    throw GeoBox_Error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" + get_info_log_string());
  }

  m_shader_program = glCreateProgram();
  glAttachShader(m_shader_program, vertex_shader);
  glAttachShader(m_shader_program, fragment_shader);
  glLinkProgram(m_shader_program);
  glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(m_shader_program, max_info_log_length, &actual_info_log_length, info_log.data());
    throw GeoBox_Error("ERROR::SHADER::PROGRAM::LINK_FAILED\n" + get_info_log_string());
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
}

Shader::~Shader() { glDeleteProgram(m_shader_program); }

void Shader::use() const { glUseProgram(m_shader_program); }

int Shader::get_uniform_location(std::string_view uniform_name) const {
  return glGetUniformLocation(m_shader_program, uniform_name.data());
}

template <typename T> std::function<void(const T &)> Shader::get_uniform_setter(std::string_view /*uniform_name*/) {
  throw GeoBox_Error("Not implemented");
}

template <> std::function<void(const glm::vec3 &)> Shader::get_uniform_setter(std::string_view uniform_name) {
  int uniform_location = get_uniform_location(uniform_name);
  return [uniform_location](const glm::vec3 &v) { glUniform3f(uniform_location, v.x, v.y, v.z); };
}

template <> std::function<void(const glm::mat4 &)> Shader::get_uniform_setter(std::string_view uniform_name) {
  int uniform_location = get_uniform_location(uniform_name);
  return
      [uniform_location](const glm::mat4 &m) { glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(m)); };
}

template <> std::function<void(const glm::mat3 &)> Shader::get_uniform_setter(std::string_view uniform_name) {
  int uniform_location = get_uniform_location(uniform_name);
  return
      [uniform_location](const glm::mat3 &m) { glUniformMatrix3fv(uniform_location, 1, GL_FALSE, glm::value_ptr(m)); };
}

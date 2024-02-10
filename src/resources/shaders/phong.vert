#version 330 core
layout(location = 0) in vec3 a_vertex_position;
layout(location = 1) in vec3 a_vertex_normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat3 normal_matrix;

out vec3 vertex_position;
out vec3 vertex_normal;

void main() {
  gl_Position = projection_matrix * view_matrix * model_matrix * vec4(a_vertex_position, 1.0f);
  vertex_position = vec3(model_matrix * vec4(a_vertex_position, 1.0f));
  vertex_normal = normal_matrix * a_vertex_normal;
}

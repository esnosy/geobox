#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_vertex_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertex_normal;

void main() {
  gl_Position = projection * view * model * vec4(a_pos, 1.0);
  vertex_normal = a_vertex_normal;
}
#version 330 core

in vec3 vertex_normal;
out vec4 frag_color;

uniform vec4 object_color;

void main() {
  vec3 light_position = vec3(10.0f, 10.0f, 10.0f);
  float light_intensity = 0.5f;
  float d = dot(vertex_normal, light_position);
  float c = max(d * light_intensity, 0);
  frag_color = vec4(vec3(c), 1.0f);
}
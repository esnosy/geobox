#version 330 core

uniform vec3 object_color;
uniform vec3 light_color;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec4 frag_color;

void main() {
  vec3 light_position = vec3(10.0f, 10.0f, 10.0f);
  float light_intensity = 0.5f;
  float ambient_strength = 0.05;
  vec3 ambient = ambient_strength * light_color;

  vec3 light_direction = normalize(light_position - vertex_position);
  vec3 diffuse = light_color * max(dot(vertex_normal, light_direction), 0.0);
  vec3 result = (ambient + diffuse) * object_color;
  frag_color = vec4(result, 1.0);
}
#version 330 core

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 camera_position;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec4 frag_color;

void main() {
  vec3 light_position = vec3(10.0f, 10.0f, 10.0f);
  float light_intensity = 0.5f;
  float ambient_strength = 0.05;
  float specular_strength = 0.5f;

  vec3 light_direction = normalize(light_position - vertex_position);
  vec3 view_dir = normalize(camera_position - vertex_position);
  vec3 reflect_dir = reflect(-light_direction, vertex_normal);

  vec3 ambient = ambient_strength * light_color;
  vec3 diffuse = light_color * max(dot(vertex_normal, light_direction), 0.0);
  vec3 specular = specular_strength * light_color * pow(max(dot(view_dir, reflect_dir), 0.0), 32);
  vec3 result = (ambient + diffuse + specular) * object_color;
  frag_color = vec4(result, 1.0);
}

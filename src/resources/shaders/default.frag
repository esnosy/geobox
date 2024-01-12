#version 330 core

in vec3 vertexNormal;
out vec4 FragColor;

uniform vec4 object_color;

void main() {
  vec3 lightPosition = vec3(10.0f, 10.0f, 10.0f);
  float lightIntensity = 0.5f;
  float d = dot(vertexNormal, lightPosition);
  float c = max(d * lightIntensity, 0);
  FragColor = vec4(vec3(c), 1.0f);
}
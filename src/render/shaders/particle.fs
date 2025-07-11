#version 330 core
out vec4 FragColor;

uniform float radiusSquared;
uniform vec3 center;
uniform float aspectRatio;

in vec3 pos;

void main() {
  vec3 diff = center - pos;
  diff.x *= aspectRatio;

  if(dot(diff, diff) <= radiusSquared) {
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  }
  else {
    FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  }
}

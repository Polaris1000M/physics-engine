#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (locaiton = 2) in vec3 aCenter;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 color;
out vec3 center;
out vec3 pos;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0f);

  pos = aPos;
  center = aCenter;
  color = aColor;
}

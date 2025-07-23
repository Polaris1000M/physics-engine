#version 330 core
out vec4 FragColor;

in vec3 color;
in float ID;

vec3 hashColor(float id) {
    float r = fract(sin(id * 12.9898) * 43758.5453);
    float g = fract(sin(id * 78.233) * 96211.3453);
    float b = fract(sin(id * 39.425) * 57321.1234);
    return vec3(r, g, b);
}

void main()
{
  FragColor = vec4(hashColor(ID), 1.0);
}


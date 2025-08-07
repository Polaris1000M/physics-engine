#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in mat4 model;
layout (location = 5) in vec3 aColor;
layout (location = 6) in vec3 aNormal;

uniform mat4 vp;
uniform mat4 shadowVP;

out vec3 color;
out vec3 normal;
out vec3 FragPos;
out vec3 ShadowCoord;

void main()
{
  gl_Position = vp * model * vec4(aPos, 1.0);

  color = aColor;
  mat3 normalMatrix = transpose(inverse(mat3(model)));
  normal = normalize(normalMatrix * aNormal);
  FragPos = vec3(model * vec4(aPos, 1.0));
  vec4 shadowCoord4 = shadowVP * vec4(FragPos, 1.0);
  ShadowCoord = (shadowCoord4.xyz / shadowCoord4.w) * 0.5 + 0.5;
}

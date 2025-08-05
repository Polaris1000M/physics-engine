#version 330 core

uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform sampler2D depthMap;

in vec3 color;
in vec3 normal;
in vec3 FragPos;
in vec3 ShadowCoord;

out vec4 FragColor;

void main()
{
  float ambientStrength = 0.6;
  vec3 ambient = ambientStrength * lightColor;

  vec3 norm = normalize(normal);
  float diff = max(dot(norm, -lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  float specularStrength = 0.5;
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * lightColor;

  float visibility = 1.0;
  float bias = 0.005;
  float shadowDepth = texture(depthMap, ShadowCoord.xy).r;
  if(shadowDepth < ShadowCoord.z - bias)
  {
    visibility = 0.5;
  }

  vec3 result = (ambient + visibility * (diffuse + specular)) * color;
  FragColor = vec4(result, 1.0);
}


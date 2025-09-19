#version 330 core

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform sampler2DShadow depthMap;

in vec3 color;
in vec3 normal;
in vec3 FragPos;
in vec3 ShadowCoord;

out vec4 FragColor;

vec2 poissonDisk[16] = vec2[]
(
     vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
     vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
     vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
     vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
     vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420),
     vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
     vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590),
     vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790)
);

void main()
{
    float ambientStrength = 0.8;
    float normalStrength = 0.4;
    float specularStrength = 0.2;

    vec3 ambient = vec3(ambientStrength);

    vec3 norm = normalize(normal);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = vec3(diff * normalStrength);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = vec3(specularStrength * spec);

    bool isFront = gl_FrontFacing;
    if(!isFront)
    {
        vec3 result = (ambient + diffuse + specular) * color;
        FragColor = vec4(result, 1.0);
        return;
    }

    float bias = 0.001;
    vec3 biasedShadowCoord = ShadowCoord;
    biasedShadowCoord.z -= bias;
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int i = 0; i < 16; i++)
    {
        vec3 sampleCoord = vec3(biasedShadowCoord.xy + poissonDisk[i] * texelSize * 2.0, biasedShadowCoord.z);
        shadow += texture(depthMap, sampleCoord);
    }
    shadow /= 16;

    vec3 result = (ambient + shadow * (diffuse + specular)) * color;
    FragColor = vec4(result, 1.0);
}


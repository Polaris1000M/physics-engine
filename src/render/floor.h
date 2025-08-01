#ifndef FLOOR_H
#define FLOOR_H

#include <glad/glad.h>
#include "shader.h"
#include "camera.h"
#include <cglm/cglm.h>

typedef struct Floor
{
    unsigned int VAO;
    unsigned int VBO;
    float mesh[36];
    Shader shader;
} Floor;

void floorInit(Floor* f);

void floorRender(Floor* f, Camera* c, vec3 lightPos);

#endif

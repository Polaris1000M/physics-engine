/*
 * shadow.h
 *
 * Generates shadows through shadow mapping based on center of camera frustum
 */

#ifndef SHADOW_H
#define SHADOW_H

#include <cglm/cglm.h>

#include "camera.h"
#include "shader.h"

typedef struct Shadow
{
    float dist;  // the distance between the near and far planes in the camera
    float padding;  // padding between camera frustum and shadow mapping box to
                    // avoid recomputing shadow map on every frame

    vec3 corners[8];  // 8 corners of the camera frustum
    vec3 lightDir;    // direction of the light
    vec3 lightDelta;  // vector to add to frustum center to find light position
    vec3
        lightPos;  // position of the light to capture all of the camera frustum
    vec3 frustumCenter;  // center of the camera frustum
    vec3 min;
    vec3 max;

    mat4 view;
    mat4 projection;  // orthographic projection matrix for shadow mapping
    mat4 vp;

    // specifies dimensions of shadow map
    unsigned int SHADOW_WIDTH;
    unsigned int SHADOW_HEIGHT;

    unsigned int FBO;
    unsigned int depthMap;

    Shader shader;  // shader program for shadow
} Shadow;

unsigned int shadowInit(Shadow* s, Camera* c, vec3 lightDir);

// update shadow data based on camera orientation
void shadowUpdate(Shadow* s, Camera* c);

void shadowPrint(Shadow* s);

#endif

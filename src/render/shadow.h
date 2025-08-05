#ifndef SHADOW_H
#define SHADOW_H

#include "camera.h"
#include "shader.h"
#include <cglm/cglm.h>

typedef struct Shadow
{
  float dist; // the distance between the near and far planes in the camera
  float padding;

  vec3 corners[8];     // 8 corners of the camera frustum
  vec3 lightDir;       // direction of the light
  vec3 lightDelta;     // vector to add to frustum center to find light position
  vec3 lightPos;       // position of the light to capture all of the camera frustum
  vec3 frustumCenter;  // center of the camera frustum
  vec3 min;
  vec3 max;

  mat4 view;
  mat4 projection;
  mat4 vp;
  mat4 biasVP;

  unsigned int SHADOW_WIDTH;
  unsigned int SHADOW_HEIGHT;

  unsigned int FBO;
  unsigned int depthMap;

  Shader shader; // shader program for shadow
} Shadow;

unsigned int shadowInit(Shadow* s, Camera* c, vec3 lightDir);

void shadowUpdate(Shadow* s, Camera* c);

void shadowPrint(Shadow* s);

void shadowSave();

#endif

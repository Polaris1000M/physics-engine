#include "object.h"
#include <stdlib.h>
#include <cglm/cglm.h>
#include "../simulation.h"
#include "particle.h"
#include "sphere.h"

const char* OBJECT_NAMES[] = {"sphere", "cube", "pyramid"};

void objectInit(Object* o, ObjectType type, float size, float mass, vec3 position, vec3 color)
{
  o->type = type;
  o->size = size;
  o->mass = mass;
  glm_vec3_copy(position, o->position);
  glm_vec3_copy(color, o->color);
  glm_vec3_copy(GLM_VEC3_ZERO, o->orientation);
}

void objectPrint(Object* o)
{
  printf("type: %s\n", OBJECT_NAMES[o->type]);
  printf("size: %f\n", o->size);
  printf("mass: %f\n", o->mass);
  printf("position: (%f, %f, %f)\n", o->position[0], o->position[1], o->position[2]);
  printf("color: (%f, %f, %f)\n", o->color[0], o->color[1], o->color[2]);
  printf("orientation: (%f, %f, %f)\n", o->orientation[0], o->orientation[1], o->orientation[2]);
}

void objectVertices(Object* o, float* vertices)
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      int idx = i * 4 + j;
      if(j == i)
      {
        if(i == 3)
        {
          vertices[idx] = 1.0f;
        }
        else
        {
          vertices[idx] = o->size / 0.5f;
        }
      }
      else
      {
        vertices[idx] = 0.0f;
      }
    }
  }

  vertices[12] = o->position[0];
  vertices[13] = o->position[1];
  vertices[14] = o->position[2];

  for(int i = 16; i < 19; i++)
  {
    vertices[i] = o->color[i - 16];
  }
}

unsigned int objectVerticesSize()
{
  // 4x4 float matrix and 3 floats for color
  return 19;
}

#include "object.h"
#include <stdlib.h>
#include <cglm/cglm.h>
#include "../simulation.h"

const char* OBJECT_NAMES[] = {"floor", "sphere", "cube", "tetrahedron"};

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
  printf("position:\n");
  glm_vec3_print(o->position, stdout);
  printf("color:\n");
  glm_vec3_print(o->color, stdout);
  printf("orientation:\n");
  glm_vec3_print(o->orientation, stdout);
}

void objectVertices(Object* o, float* vertices)
{
  mat4 res;
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      if(i != j)
      {
        scale[i][j] = o->size;
      }
      else
      {
        scale[i][j] = 0.0f;
      }
    }
  }
  scale[3][3] = 1.0f;

  glm_translate(res, o->position);

  mat4 rot;
  glm_euler((vec3) {glm_rad(object->orientation[0]), glm_rad(object->orientation[1]), glm_rad(object->orientation[2])}, rot);

  glm_mat4_mul(res, rot, res);


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


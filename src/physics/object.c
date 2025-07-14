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

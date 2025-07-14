#include "object.h"
#include <stdlib.h>
#include <cglm/cglm.h>
#include "../simulation.h"
#include "particle.h"
#include "sphere.h"

void objectInit(Object* o, ObjectType type, float size, float mass, vec3 position, vec3 color)
{
  o->type = type;
  o->size = size;
  o->mass = mass;
  glm_vec3_copy(position, o->position);
  glm_vec3_copy(color, o->color);
  glm_vec3_copy(GLM_VEC3_ZERO, o->orientation);
}

#include "object.h"
#include <stdlib.h>
#include <cglm/cglm.h>

void objectInit(Object* o, ObjectType type, float size, float mass, vec3 position, vec3 color) {
  o->type = type;
  o->size = size;
  o->mass = mass;
  glm_vec3_copy(o->position, position);
  glm_vec3_copy(o->color, color);
  glm_vec3_copy(o->velocity, GLM_VEC3_ZERO);
}

#ifndef OBJECT_H
#define OBJECT_H

#include <cglm/cglm.h>
#include <stdlib.h>

#define OBJECT_TYPES 3

typedef enum {
  OBJECT_PARTICLE,
  OBJECT_CUBE,
  OBJECT_TRIANGLE
} ObjectType;

typedef struct Object {
  ObjectType type;
  float size;
  float mass;
  vec3 position;
  vec3 color;
  vec3 velocity;
} Object;

void objectInit(Object* o, ObjectType type, float size, float mass, vec3 position, vec3 color);

#endif

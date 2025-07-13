#ifndef OBJECT_H
#define OBJECT_H

#include <cglm/cglm.h>

#define OBJECT_TYPES 3

typedef enum {
  OBJECT_PARTICLE,
  OBJECT_SPHERE,
  OBJECT_CUBE,
  OBJECT_TRIANGLE
} ObjectType;

typedef struct Simulation Simulation;

typedef struct Object {
  ObjectType type;
  float size;
  float mass;
  vec3 position;
  vec3 color;
  vec3 velocity;
  vec3 orientation;

  void (*render)(Simulation*, vec3, vec3, vec3, float);

} Object;

void objectInit(Object* o, ObjectType type, float size, float mass, vec3 position, vec3 color);

#endif

#ifndef OBJECT_H
#define OBJECT_H

#include <cglm/cglm.h>

#define OBJECT_TYPES 3

extern const char* OBJECT_NAMES[3];

typedef enum
{
  SPHERE,
  CUBE,
  PYRAMID 
} ObjectType;

// represents an object in the simulation
typedef struct Object
{
  ObjectType type;
  float size;
  float mass;
  vec3 position;
  vec3 color;
  vec3 orientation; // euler angles representing rotation
} Object;

// initializes an object
void objectInit(Object* o, ObjectType type, float size, float mass, vec3 position, vec3 color);

// prints an object
void objectPrint(Object* o);

#endif

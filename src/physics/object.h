/*
 * object.h
 *
 * Describes the objects in the simulation, with the same object type for each type of simulation object
 *
 * Besides provided functions, each object should support:
 * - objectMesh method to generate a default mesh
 * - objectMeshSize to specify the size of the mesh
 * - objectBind method to specify OpenGL boilerplate in instancing each default mesh
 *
 * No specific methods are needed for each object to compute how to store per frame data, since the information is identical for each object: model matrix and color
 */

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

// generates the vertex data for each object comprised of the model matrix and color data
void objectVertices(Object* o, float* vertices);

// returns the size of an object's per instance data
unsigned int objectVerticesSize();

#endif

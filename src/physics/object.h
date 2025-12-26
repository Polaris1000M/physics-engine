/*
 * object.h
 *
 * Generic object struct for simulation
 * Stores position, color, and orientation
 * Can generate each object's model matrix
 *
 * Each type of object (sphere, cube, etc.) should individually support their
 * own:
 * - objectMesh method to generate a default mesh
 * - objectMeshSize method to specify the size of the mesh
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <cglm/cglm.h>

#include "cJSON.h"

#define OBJECT_TYPES 4

extern const char* OBJECT_NAMES[OBJECT_TYPES];

typedef enum
{
    FLOOR,
    SPHERE,
    CUBE,
    TETRAHEDRON
} ObjectType;

// represents an object in the simulation
typedef struct Object
{
    ObjectType type;
    float size;
    float mass;
    vec3 color;

    int staticPhysics;  // flag indicating whether to ignore physics for object

    vec3 lastPosition;  // prior position for Verlet integration
    vec3 position;
    vec3 linearAcceleration;

    vec3 angularVelocity;
    versor orientation;  // quaternion representing orientation
    vec3 angularAcceleration;
} Object;

// initializes an object
void objectInit(Object* o, ObjectType type, float size, float mass,
                vec3 position, vec3 color);

// prints an object
void objectPrint(Object* o);

// generates and stores model matrix and color data
void objectVertices(Object* o, float* vertices);

// returns the size of an object's per instance data
unsigned int objectVerticesSize();

// converts object data into JSON
cJSON* objectToJSON(Object* o);

#endif


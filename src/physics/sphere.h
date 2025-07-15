#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"

// fills array with vertex data assuming object is a sphere
void sphereVertices(float* vertices, Object* s);

// computes the number of vertices in a single sphere object
unsigned int sphereVertexCount();

#endif

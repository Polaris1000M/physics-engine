#ifndef PYRAMID_H
#define PYRAMID_H

#include "object.h"

// fills array with vertices assuming object is a pyramid
void pyramidVertices(float* vertices, Object* p);

// computes the number of vertices in a single pyramid object
unsigned int pyramidVertexCount();

#endif

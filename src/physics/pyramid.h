#ifndef PYRAMID_H
#define PYRAMID_H

#include "object.h"

// fills array with vertices assuming object is a pyramid
void pyramidMesh(float* vertices);

// computes the number of vertices in a single pyramid object
unsigned int pyramidMeshSize();

unsigned int pyramidBindMesh(float* vertices);

#endif

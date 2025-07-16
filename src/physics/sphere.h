#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"

// generates the default mesh for a sphere
void sphereMesh(float* vertices);

// computes the number of vertices in the sphere mesh
unsigned int sphereMeshSize();

void sphereBindMesh(unsigned int VAO, float* vertices);

#endif

#ifndef CUBE_H
#define CUBE_H

#include "object.h"

// 6 faces
// 2 triangles per face
// 9 coordinates per triangle
// 9 color coordinates per triangle
#define CUBE_VERTEX_COUNT 216

// fills array with vertices assuming object is a cube 
void cubeMesh(float* vertices);

// computes the number of vertices in a single cube object
unsigned int cubeMeshSize();

// creates VAO from sphere mesh data
unsigned int cubeBindMesh(float* vertices);

#endif

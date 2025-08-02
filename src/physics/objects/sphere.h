#ifndef SPHERE_H
#define SPHERE_H

// generates the UV mesh for a sphere
void sphereUVMesh(float* vertices);

// computes the number of floats in the UV sphere mesh
unsigned int sphereUVMeshSize();

// generate mesh for icosphere
void sphereIcoMesh(float* vertices);

// computes number of floats in icosphere
unsigned int sphereIcoMeshSize();

#endif

#ifndef SPHERE_H
#define SPHERE_H

// generates the UV mesh for a sphere
void sphereUVMesh(float* mesh);

// computes the number of floats in the UV sphere mesh
unsigned int sphereUVMeshSize();

// generate mesh for icosphere
void sphereIcoMesh(float* mesh);

// computes number of floats in icosphere
unsigned int sphereIcoMeshSize();

#endif


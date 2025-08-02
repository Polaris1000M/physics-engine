#ifndef TETRAHEDRON_H
#define TETRAHEDRON_H

// fills array with vertices assuming object is a tetrahedron 
void tetrahedronMesh(float* vertices);

// computes the number of vertices in a single tetrahedron object
unsigned int tetrahedronMeshSize();

unsigned int tetrahedronBindMesh(float* vertices);

#endif

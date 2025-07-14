#ifndef SPHERE_H
#define SPHERE_H

#include <cglm/cglm.h>
#include "../simulation.h"

typedef struct Sphere
{
  float radius;
  float mass;
  vec3 position;
  unsigned int sectors; // number of sectors
  unsigned int stacks; // number of stacks
  unsigned int n; // number of vertices in sphere
} Sphere;

// initialize a new sphere
void sphereInit(Sphere* s, float radius, float mass, vec3 position, unsigned int sectors, unsigned int stacks);

// return a pointer to an array of sphere vertices
float* sphereVertices(Sphere* s);

void renderSphere(Simulation* sim, vec3 position, vec3 orientation, vec3 color, float radius);

#endif

#ifndef PARTICLE_H
#define PARTICLE_H

#include <cglm/cglm.h>
#include "../simulation.h"

typedef struct Particle {
  float radius;
} Particle;

// void particleVertices(Particle* p, float* vertices);

// void particleFormat(Particle* p);

void renderParticle(Simulation* sim, vec3 position, vec3 orientation, vec3 color, float radius);

#endif

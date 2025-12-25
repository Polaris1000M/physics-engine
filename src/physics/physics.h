#ifndef PHYSICS_H
#define PHYSICS_H

#define PHYSICS_DT (1.0f / 60.0f)

typedef struct Simulation Simulation;

void physicsUpdate(Simulation* sim);

#endif

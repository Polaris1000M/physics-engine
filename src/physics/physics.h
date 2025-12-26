#ifndef PHYSICS_H
#define PHYSICS_H

#define PHYSICS_DT 0.0166666666667
#define PHYSICS_DT2 0.000277777777778

typedef struct Simulation Simulation;

// update object positions
void physicsUpdate(Simulation* sim);

#endif


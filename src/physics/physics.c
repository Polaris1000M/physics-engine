#include "physics.h"
#include "../simulation.h"
#include "cglm/vec3.h"
#include <cglm/cglm.h>

// finds current accelerations for each object in the simulation
void resolveForces(Simulation* sim)
{
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        for (int i = 0; i < sim->objectCounts[type]; i++)
        {
            glm_vec3_copy((vec3) { 0, -1, 0 }, sim->objects[type][i].linearAcceleration);
        }
    }
}

// use verletIntegration to update object positions and orientations
void verletIntegration(Simulation* sim, float deltaTime)
{
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        for (int i = 0; i < sim->objectCounts[type]; i++)
        {
            Object* object = sim->objects[type] + i;
            vec3 deltaPosition;
            glm_vec3_sub(object->position, object->lastPosition, deltaPosition);

            glm_vec3_scale(object->linearAcceleration, deltaTime * deltaTime, object->linearAcceleration);

            glm_vec3_copy(object->position, object->lastPosition);
            glm_vec3_addadd(deltaPosition, object->linearAcceleration, object->position);
        }
    }
}

void physicsUpdate(Simulation *sim, float deltaTime)
{
    resolveForces(sim);
    verletIntegration(sim, deltaTime);
}


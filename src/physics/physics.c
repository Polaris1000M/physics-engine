#include "physics.h"
#include "../simulation.h"
#include "object.h"
#include "cglm/vec3.h"
#include <cglm/cglm.h>

// finds current accelerations for each object in the simulation
void resolveForces(Simulation* sim)
{
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        for (int i = 0; i < sim->objectCounts[type]; i++)
        {
            if (sim->objects[type][i].staticPhysics)
            {
                continue;
            }

            glm_vec3_copy((vec3) { 0, sim->gravity, 0 }, sim->objects[type][i].linearAcceleration);
            // glm_vec3_copy((vec3) { 0, 0, 0 }, sim->objects[type][i].linearAcceleration);
        }
    }
}

// use verletIntegration to update object positions and orientations
void verletIntegration(Simulation* sim)
{
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        for (int i = 0; i < sim->objectCounts[type]; i++)
        {
            if (sim->objects[type][i].staticPhysics)
            {
                continue;
            }

            Object* object = sim->objects[type] + i;
            vec3 deltaPosition;
            glm_vec3_sub(object->position, object->lastPosition, deltaPosition);

            glm_vec3_scale(object->linearAcceleration, sim->physicsDeltaTime * sim->physicsDeltaTime, object->linearAcceleration);

            glm_vec3_copy(object->position, object->lastPosition);
            glm_vec3_addadd(deltaPosition, object->linearAcceleration, object->position);
        }
    }
}

void physicsUpdate(Simulation *sim)
{
    resolveForces(sim);
    verletIntegration(sim);
}


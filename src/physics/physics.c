#include "physics.h"

#include <cglm/cglm.h>

#include "../simulation.h"
#include "object.h"

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

            glm_vec3_copy((vec3){0, sim->gravity, 0},
                          sim->objects[type][i].linearAcceleration);
            // glm_vec3_copy((vec3) { 0, 0, 0 },
            // sim->objects[type][i].linearAcceleration);
        }
    }
}

// use Verlet integration to update object positions
void linearUpdate(Object* object)
{
    vec3 deltaPosition;
    glm_vec3_sub(object->position, object->lastPosition, deltaPosition);

    glm_vec3_scale(object->linearAcceleration, PHYSICS_DT2,
                   object->linearAcceleration);

    glm_vec3_copy(object->position, object->lastPosition);
    glm_vec3_addadd(deltaPosition, object->linearAcceleration,
                    object->position);
}

// use sympletic Euler to update angular orientation
void angularUpdate(Object* object)
{
    // update angular velocity
    glm_vec3_muladds(object->angularAcceleration, PHYSICS_DT,
                     object->angularVelocity);

    // calculate angle and rotation axis
    float angle = glm_vec3_norm(object->angularVelocity);
    vec3 axis;
    glm_vec3_scale(object->angularVelocity, angle, axis);
    angle *= PHYSICS_DT;

    versor deltaOrientation;
    glm_quatv(deltaOrientation, angle, axis);

    glm_quat_mul(deltaOrientation, object->orientation, object->orientation);
    glm_quat_normalize(object->orientation);
}

void physicsUpdate(Simulation* sim)
{
    resolveForces(sim);

    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        for (int i = 0; i < sim->objectCounts[type]; i++)
        {
            if (sim->objects[type][i].staticPhysics)
            {
                continue;
            }

            linearUpdate(sim->objects[type] + i);
            angularUpdate(sim->objects[type] + i);
        }
    }
}


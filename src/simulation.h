/*
 * simulation.h
 *
 * Core simulation struct which manages objects, object interactions, and object
 * rendering
 */

#ifndef SIMULATION_H
#define SIMULATION_H

#include <glad/glad.h>  // must be included first
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "physics/object.h"
#include "render/camera.h"
#include "render/shader.h"
#include "render/shadow.h"
#include "render/text.h"

typedef struct Simulation
{
    /* SIMULATION MANAGEMENT VARIABLES */
    const char* configPath;  // used to reset the simulation
    GLFWwindow* window;
    int initialized;  // whether the simulation has already been initialized for
                      // restarting purposes
    unsigned int
        objectCounts[OBJECT_TYPES];  // the number of each type of object

    /* PHYSICS VARIABLES */
    float gravity;
    void (*collisionTable[OBJECT_TYPES][OBJECT_TYPES])(
        float*);  // table of function pointers for collision resolution
    Object* objects[OBJECT_TYPES];  // holds object rigid body data for physics
                                    // calculations

    /* METRICS */
    float avgFPS;               // average FPS of simulation
    unsigned long long frames;  // number of frames
    float lastTime;  // last time simulation loop ran, used to calculate FPS

    /* RENDERING VARIABLES */
    Shader shader;
    Shadow shadow;
    float lightDir[3];
    Camera camera;
    Text text;

    // object data (model matrix and color)
    unsigned int objectVBOs[OBJECT_TYPES];  // VBOs for object data
    unsigned int VAOs[OBJECT_TYPES];        // VAOs for each type of object
    unsigned int
        objectSizes[OBJECT_TYPES];  // the number of floats in all of the object
                                    // data for each object type
    float* objectData[OBJECT_TYPES];  // buffer with per object rendering data
                                      // (model matrix and color)

    // meshes
    unsigned int meshVBOs[OBJECT_TYPES];   // VBOs for each of the object meshes
    unsigned int meshSizes[OBJECT_TYPES];  // the number of floats in a single
                                           // mesh of each object
    float* meshes[OBJECT_TYPES];  // default meshes for each object type

} Simulation;

// initialize the simulation
unsigned int simulationInit(Simulation* sim, const char* configPath);

// updates the positions of the objects in the simulation based on time passed
void simulationUpdate(Simulation* sim);

// starts the simulation
void simulationStart(Simulation* sim);

// save the current state of the simulation into JSON format
void simulationSave(Simulation* sim);

#endif


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

typedef struct Simulation
{
    GLFWwindow* window;
    unsigned int WINDOW_HEIGHT;
    unsigned int WINDOW_WIDTH;

    float gravity;
    float lightDir[3];
    float lastTime;   // last time render loop was called
    float timeRatio;  // multiplied by amount of real time passing to produce
                      // amount of simulation time passed

    Shader shader;
    Camera camera;
    Shadow shadow;

    // arrays holding all objects
    Object* objects[OBJECT_TYPES];

    // the number of each type of object
    unsigned int objectCounts[OBJECT_TYPES];

    // the number of floats in all of the objects for each object type
    unsigned int objectSizes[OBJECT_TYPES];

    // VBO IDs for objects
    unsigned int objectVBOs[OBJECT_TYPES];

    // buffer with per object rendering data (model matrix and
    // color)
    float* objectData[OBJECT_TYPES];

    // default meshes for each object type
    float* meshes[OBJECT_TYPES];

    // the number of floats in a single mesh of each object
    unsigned int meshSizes[OBJECT_TYPES];

    // VBOs for each of the object meshes
    unsigned int meshVBOs[OBJECT_TYPES];

    // VAOs for each type of object
    unsigned int VAOs[OBJECT_TYPES];

} Simulation;

// initialize the simulation
unsigned int simulationInit(Simulation* sim, const char* configPath);

// updates the positions of the objects in the simulation based on time passed
void simulationUpdate(Simulation* sim);

// renders the current state of the simulation
void simulationRender(Simulation* sim);

// starts the simulation
void simulationStart(Simulation* sim);

// prints the contents of the simulation for debugging
void simulationPrint(Simulation* sim);

#endif

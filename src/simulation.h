#ifndef SIMULATION_H
#define SIMULATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "physics/object.h"
#include "render/shader.h"
#include "render/camera.h"
#include "render/texture.h"

#define MAX_OBJECTS 100

typedef struct Simulation {
  unsigned int WINDOW_HEIGHT;
  unsigned int WINDOW_WIDTH;
  unsigned int n;
  GLFWwindow* window;
  float gravity;
  Object objects[MAX_OBJECTS];
} Simulation;

// initialize the simulation
void simulationInit(Simulation* s, unsigned int n);

// updates the positions of the objects in the simulation based on time passed
void simulationUpdate(Simulation* s, float deltaTime);

// renders the current state of the simulation
void simulationRender(Simulation* s);

// starts the simulation
void simulationStart(Simulation* s);

#endif

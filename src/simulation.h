#ifndef SIMULATION_H
#define SIMULATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "physics/object.h"
#include "physics/sphere.h"
#include "physics/cube.h"
#include "physics/pyramid.h"
#include "render/shader.h"
#include "render/camera.h"

typedef struct Simulation {
  GLFWwindow* window;
  unsigned int WINDOW_HEIGHT;
  unsigned int WINDOW_WIDTH;

  float gravity;
  float lastTime; // last time render loop was called
  float timeRatio; // multiplied by amount of real time passing to produce amount of simulation time passed
  
  Shader shader;
  Camera camera;

  // the number of each type of object
  unsigned int counts[OBJECT_TYPES];

  // arrays holding all objects
  Object* objects[OBJECT_TYPES];

  // the number of vertices stored on the VBOs of each object 
  unsigned int vertexCounts[OBJECT_TYPES];

  // the number of floats in a single instance of the object
  unsigned int singleVertexCounts[OBJECT_TYPES];

  // buffer with all the vertices of the objects
  float* vertices[OBJECT_TYPES];

  // methods to generate vertices
  void (*generateVertices[OBJECT_TYPES])(float*, Object*);

  unsigned int VAOs[OBJECT_TYPES];
  unsigned int VBOs[OBJECT_TYPES];
  unsigned int UBOs[OBJECT_TYPES];

} Simulation;

// initialize the simulation
int simulationInit(Simulation* sim, const char* configPath);

// updates the positions of the objects in the simulation based on time passed
void simulationUpdate(Simulation* sim, float deltaTime);

// renders the current state of the simulation
void simulationRender(Simulation* sim);

// starts the simulation
void simulationStart(Simulation* sim);

// prints the contents of the simulation for debugging
void simulationPrint(Simulation* sim);

#endif

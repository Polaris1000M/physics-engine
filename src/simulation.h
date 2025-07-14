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

#define MAX_OBJECTS 100

typedef struct Sphere Sphere;
typedef struct Cube Cube;
typedef struct Pyramid Pyramid;

typedef struct Simulation {
  GLFWwindow* window;
  unsigned int WINDOW_HEIGHT;
  unsigned int WINDOW_WIDTH;

  float gravity;
  float lastTime; // last time render loop was called
  float timeRatio; // multiplied by amount of real time passing to produce amount of simulation time passed
  
  Shader shader;
  Shader particleShader;
  Camera camera;

  unsigned int n;
  Object objects[MAX_OBJECTS];

  // the number of each type of object
  unsigned int sphereCount;
  unsigned int cubeCount;
  unsigned int pyramidCount;

  // arrays holding all objects
  Sphere* spheres;
  Cube* cubes;
  Pyramid* pyramids;

  unsigned int sphereVertexCount;
  unsigned int cubeVertexCount;
  unsigned int pyramidVertexCount;

  // arrays holding vertices
  float* sphereVertices;
  float* cubeVertices;
  float* pyramidVertices;

  unsigned int sphereVAO;
  unsigned int cubeVAO;
  unsigned int pyramidVAO;
  unsigned int sphereVBO;
  unsigned int cubeVBO;
  unsigned int pyramidVBO;
} Simulation;

// initialize the simulation
void simulationInit(Simulation* sim, const char* configPath);

// updates the positions of the objects in the simulation based on time passed
void simulationUpdate(Simulation* sim, float deltaTime);

// renders the current state of the simulation
void simulationRender(Simulation* sim);

// starts the simulation
void simulationStart(Simulation* sim);

#endif

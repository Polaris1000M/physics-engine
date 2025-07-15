#include "simulation.h"
#include "utils/parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// initializes OpenGL and GLFW boilerplate
int openglInit(Simulation* sim)
{
  const char* openglInitErrorMessage = "ERROR::OPENGL::INITIALIZATION_ERROR: failed to initialize OpenGL and GLFW\n";

  if (!glfwInit())
  {
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  sim->WINDOW_WIDTH = 800;
  sim->WINDOW_HEIGHT = 600;
  sim->window = glfwCreateWindow(sim->WINDOW_WIDTH, sim->WINDOW_HEIGHT, "ParticleSimulator", NULL, NULL);
  if(!sim->window)
  {
    glfwTerminate();
    printf("%s", openglInitErrorMessage);
    return 1;
  }

  glfwMakeContextCurrent(sim->window);

  if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
  {
    printf("%s", openglInitErrorMessage);
    return 1;
  }

  return 0;
}

// initializes object data based on config
int objectsInit(Simulation* sim, const char* configPath)
{
  // parse config file
  char* configData = parseFile(configPath, "CONFIG");
  if(!configData)
  {
    return 1;
  }
  cJSON* config = cJSON_Parse(configData);

  const cJSON* gravity = cJSON_GetObjectItemCaseSensitive(config, "gravity");
  if(!cJSON_IsNumber(gravity))
  {
    printf("ERROR::CONFIG::INVALID_GRAVITY: expected float\n");
    return 1;
  }
  sim->gravity = gravity->valuedouble;

  cJSON* rawObjects = cJSON_GetObjectItemCaseSensitive(config, "objects");
  ConfigObject* configObjects = parseConfigObjects(rawObjects);
  if(!configObjects)
  {
    return 1;
  }
  unsigned int numConfigObjects = cJSON_GetArraySize(rawObjects);
  convertConfigObjectsToObjects(numConfigObjects, configObjects, sim->counts, sim->objects);
  
  return 0;
}

// initalizes and binds vertex buffers
int buffersInit(Simulation* sim)
{
  void (*generateVertices[OBJECT_TYPES])(float*, Object*); 
  generateVertices[SPHERE] = sphereVertices;
  generateVertices[CUBE] = cubeVertices;
  generateVertices[PYRAMID] = pyramidVertices;

  sim->singleVertexCounts[SPHERE] = sphereVertexCount();
  sim->singleVertexCounts[CUBE] = cubeVertexCount();
  sim->singleVertexCounts[PYRAMID] = pyramidVertexCount();

  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    sim->vertexCounts[type] = sim->counts[type] * sim->singleVertexCounts[type];
    sim->vertices[type] = malloc(sim->vertexCounts[type] * sizeof(float));

    int idx = 0; // the index to generate new vertices in the vertices array

    for(int i = 0; i < sim->counts[type]; i++)
    {
      generateVertices[type](sim->vertices[type] + idx, sim->objects[type] + i);
      idx += sim->singleVertexCounts[type];
    }
  }

  return 0;
}

int simulationInit(Simulation* sim, const char* configPath)
{
  // OpenGL boilerplate
  if(openglInit(sim))
  {
    return 1;
  }

  // initialize objects
  if(objectsInit(sim, configPath))
  {
    return 1;
  }

  sim->lastTime = 0.0f;
  sim->timeRatio = 0.5f;

  // initialize shader programs
  shaderInit(&sim->shader, "../src/render/shaders/sphere.vs", "../src/render/shaders/sphere.fs");

  // initialize camera
  cameraInit(&sim->camera, sim->window);
  glEnable(GL_DEPTH_TEST); // draws pixel if it is not behind another pixel
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // initalize vertex and bind
  buffersInit(sim);  

  // shaderUse(&sim->shader);

  return 0;
}

void simulationUpdate(Simulation* sim, float deltaTime)
{

}

void simulationRender(Simulation* sim)
{
}

void simulationStart(Simulation* sim)
{
  while(!glfwWindowShouldClose(sim->window))
  {
    if(glfwGetKey(sim->window, GLFW_KEY_ESCAPE))
    {
      glfwSetWindowShouldClose(sim->window, 1);
    }

    cameraKeyboardCallback(&sim->camera, sim->window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update particle positions
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - sim->lastTime;
    simulationUpdate(sim, sim->timeRatio * deltaTime);

    // calculate FPS
    printf("%f\n", 1000.0f / deltaTime);

    sim->lastTime = currentTime;

    simulationRender(sim);

    glfwSwapBuffers(sim->window);

    glfwPollEvents();
  }

  glfwTerminate();
}

void simulationPrint(Simulation* sim)
{
  printf("OBJECTS\n");
  printf("Object Counts\n");
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s: %d\n", OBJECT_NAMES[type], sim->counts[type]);
  }
  printf("\n");

  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s\n", OBJECT_NAMES[type]);

    for(int i = 0; i < sim->counts[type]; i++)
    {
      objectPrint(sim->objects[type] + i);
      printf("\n");
    }
    printf("\n");
  }

  printf("VERTICES\n");
  printf("Single Vertex Counts\n");
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s: %d\n", OBJECT_NAMES[type], sim->singleVertexCounts[type]);
  }
  printf("\n");

  printf("Vertex Counts\n");
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s: %d\n", OBJECT_NAMES[type], sim->vertexCounts[type]);
  }
  printf("\n");

  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s\n", OBJECT_NAMES[type]);
    // print all vertices in a single object
    for(int i = 0; i < sim->vertexCounts[type]; i += 3)
    {
      printf("(%f, %f, %f)\n", sim->vertices[type][i], sim->vertices[type][i + 1], sim->vertices[type][i + 2]);
      // // iterate over all groups of 3 vertices in the current object
      // for(int j = 0; j < i * sim->singleVertexCounts[type]; j += 3)
      // {
      //   printf("(%f, %f, %f)\n", sim->vertices[type][j], sim->vertices[type][j + 1], sim->vertices[type][j + 2]);
      // }
    }
  }
}

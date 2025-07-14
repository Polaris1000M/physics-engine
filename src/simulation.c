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

int simulationInit(Simulation* sim, const char* configPath)
{
  // OpenGL boilerplate
  if(openglInit(sim))
  {
    return 1;
  }

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

  sim->lastTime = 0.0f;
  sim->timeRatio = 0.5f;

  // initialize shader programs
  shaderInit(&sim->shader, "../src/render/shaders/sphere.vs", "../src/render/shaders/sphere.fs");

  // initialize camera
  cameraInit(&sim->camera, sim->window);
  glEnable(GL_DEPTH_TEST); // draws pixel if it is not behind another pixel
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  shaderUse(&sim->shader);

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
  for(int i = 0; i < OBJECT_TYPES; i++)
  {
    printf("%s\n", OBJECT_NAMES[i]);

    for(int j = 0; j < sim->counts[i]; j++)
    {
      objectPrint(&sim->objects[i][j]);
      printf("\n");
    }
    printf("\n");
  }
}

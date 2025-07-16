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
  convertConfigObjectsToObjects(numConfigObjects, configObjects, sim->objectCounts, sim->objects);
  
  return 0;
}

// initalizes and binds default meshes
int buffersInit(Simulation* sim)
{
  sim->meshSizes[SPHERE] = sphereMeshSize();
  sim->meshSizes[CUBE] = cubeMeshSize();
  sim->meshSizes[PYRAMID] = pyramidMeshSize();

  void (*generateMesh[OBJECT_TYPES])(float*); 
  generateMesh[SPHERE] = sphereMesh;
  generateMesh[CUBE] = cubeMesh;
  generateMesh[PYRAMID] = pyramidMesh;

  glGenVertexArrays(OBJECT_TYPES, sim->VAOs);
  glGenBuffers(OBJECT_TYPES, sim->VBOs);

  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    sim->meshes[type] = malloc(sim->meshSizes[type] * sizeof(float));
    generateMesh[type](sim->meshes[type]);

    glBindVertexArray(sim->VAOs[type]);
    glBindBuffer(GL_ARRAY_BUFFER, sim->VBOs[type]);
    glBufferData(GL_ARRAY_BUFFER, sim->meshSizes[type] * sizeof(float), sim->meshes[type], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    sim->vertexCounts[type] = sim->objectCounts[type] * objectVerticesSize();
    sim->vertices[type] = malloc(sim->vertexCounts[type] * sizeof(float));

    for(unsigned int i = 0, idx = 0; i < sim->objectCounts[type]; i++, idx += objectVerticesSize())
    {
      objectVertices(&sim->objects[type][i], sim->vertices[type] + idx);
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

  // initalize default meshes and bind
  buffersInit(sim);  

  shaderUse(&sim->shader);

  return 0;
}

void simulationUpdate(Simulation* sim, float deltaTime)
{

}

void simulationRender(Simulation* sim)
{
  shaderUse(&sim->shader);

  mat4 view, projection;
  cameraView(&sim->camera, view);
  cameraProjection(&sim->camera, projection);
  shaderSetMatrix(&sim->shader, "view", view);
  shaderSetMatrix(&sim->shader, "projection", projection);

  for(unsigned int type = 0; type < OBJECT_TYPES; type++)
  {
    for(unsigned int i = 0, idx = 0; i < sim->objectCounts[type]; i++, idx += objectVerticesSize())
    {
      objectVertices(&sim->objects[type][i], sim->vertices[type] + idx);
    }

    glBindVertexArray(sim->VAOs[type]);

    for(unsigned int i = 1; i < 5; i++)
    {
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, objectVerticesSize(), (void*) (i * sizeof(vec4)));
      glVertexAttribDivisor(i, 1);
    }

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, objectVerticesSize(), (void*) sizeof(mat4));
    glVertexAttribDivisor(5, 1);

    glDrawElementsInstanced(GL_TRIANGLES, sim->objectCounts[type] * objectVerticesSize(), GL_UNSIGNED_INT, 0, sim->objectCounts[type]);
  }
}

void simulationStart(Simulation* sim)
{
  shaderUse(&sim->shader);

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  unsigned int size = sphereMeshSize();
  float* vertices = malloc(sphereMeshSize() * sizeof(float));
  sphereMesh(vertices);

  for(int i = 0; i < size; i += 3)
  {
    printf("(%f, %f, %f)\n", vertices[i], vertices[i + 1], vertices[i + 2]);
  }

  glBufferData(GL_ARRAY_BUFFER, sphereMeshSize() * sizeof(float), vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);

  unsigned int instanceVBO;
  glGenBuffers(1, &instanceVBO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

  float models[1600];
  for(int idx = 0; idx < 1600; idx += 16)
  {
    for(int i = 0; i < 4; i++)
    {
      for(int j = 0; j < 4; j++)
      {
        int pos = idx + i * 4 + j;
        if(j == i)
        {
          models[pos] = 1.0f;
        }
        else
        {
          models[pos] = 0.0f;
        }
      }

      models[idx + 12] = idx / 16;
      models[idx + 13] = idx / 16;
    }
  }


  glBufferData(GL_ARRAY_BUFFER, sizeof(models), models, GL_STATIC_DRAW);
  for(int i = 0; i < 4; i++)
  {
    glVertexAttribPointer(i + 1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*) (i * 4 * sizeof(float)));
    glEnableVertexAttribArray(i + 1);
    glVertexAttribDivisor(i + 1, 1);
  }

  mat4 model = GLM_MAT4_IDENTITY;
  shaderSetMatrix(&sim->shader, "model", model);

  while(!glfwWindowShouldClose(sim->window))
  {
    if(glfwGetKey(sim->window, GLFW_KEY_ESCAPE))
    {
      glfwSetWindowShouldClose(sim->window, 1);
    }

    cameraKeyboardCallback(&sim->camera, sim->window);

    mat4 view = GLM_MAT4_IDENTITY;
    mat4 projection = GLM_MAT4_IDENTITY;
    cameraView(&sim->camera, view);
    cameraProjection(&sim->camera, projection);
    shaderSetMatrix(&sim->shader, "view", view);
    shaderSetMatrix(&sim->shader, "projection", projection);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update particle positions
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - sim->lastTime;
    simulationUpdate(sim, sim->timeRatio * deltaTime);

    // calculate FPS
    printf("%f\n", 1000.0f / deltaTime);

    sim->lastTime = currentTime;

    // simulationRender(sim);
    glDrawArraysInstanced(GL_TRIANGLES, 0, sphereMeshSize() / 3, 100);

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
    printf("%s: %d\n", OBJECT_NAMES[type], sim->objectCounts[type]);
  }
  printf("\n");

  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s\n", OBJECT_NAMES[type]);

    for(int i = 0; i < sim->objectCounts[type]; i++)
    {
      objectPrint(sim->objects[type] + i);
      printf("\n");
    }
    printf("\n");
  }

  // printf("MESHES\n");
  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   printf("%d\n", sim->meshSizes[type]);
  //   for(int i = 0; i < sim->meshSizes[type]; i += 3)
  //   {
  //     printf("(%f, %f, %f)\n", sim->meshes[type][i], sim->meshes[type][i + 1], sim->meshes[type][i + 2]);
  //   }
  // }

  printf("VERTICES\n");
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s: %d\n", OBJECT_NAMES[type], sim->vertexCounts[type]);
  }
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    for(int i = 0; i < sim->objectCounts[type]; i++)
    {
      for(int j = 0; j < 19; j++)
      {
        printf("%f ", sim->vertices[type][i * objectVerticesSize() + j]);
      }
      printf("\n");
    }
    printf("\n");
  }

  // printf("Single Vertex Counts\n");
  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   printf("%s: %d\n", OBJECT_NAMES[type], sim->singleVertexCounts[type]);
  // }
  // printf("\n");

  // printf("Vertex Counts\n");
  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   printf("%s: %d\n", OBJECT_NAMES[type], sim->vertexCounts[type]);
  // }
  // printf("\n");

  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   printf("%s\n", OBJECT_NAMES[type]);
  //   // print all vertices in a single object
  //   for(int i = 0; i < sim->vertexCounts[type]; i += 3)
  //   {
  //     printf("(%f, %f, %f)\n", sim->vertices[type][i], sim->vertices[type][i + 1], sim->vertices[type][i + 2]);
  //     // // iterate over all groups of 3 vertices in the current object
  //     // for(int j = 0; j < i * sim->singleVertexCounts[type]; j += 3)
  //     // {
  //     //   printf("(%f, %f, %f)\n", sim->vertices[type][j], sim->vertices[type][j + 1], sim->vertices[type][j + 2]);
  //     // }
  //   }
  // }
}

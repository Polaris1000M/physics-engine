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

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);


  sim->WINDOW_WIDTH = mode->width;
  sim->WINDOW_HEIGHT = mode->height;
  sim->window = glfwCreateWindow(sim->WINDOW_WIDTH, sim->WINDOW_HEIGHT, "PhysicsEngine", NULL, NULL);
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

  const cJSON* light = cJSON_GetObjectItemCaseSensitive(config, "light");
  const char* lightMessage = "ERROR::CONFIG::INVALID_LIGHT: expected float array with format [<x>, <y>, <z>] for light position\n";
  if(!cJSON_IsArray(light) || cJSON_GetArraySize(light) != 3)
  {
    printf("%s\n", lightMessage);
    return 1;
  }
  for(int i = 0; i < 3; i++)
  {
    cJSON* lightCoord = cJSON_GetArrayItem(light, i);
    if(!cJSON_IsNumber(lightCoord))
    {
      printf("%s\n", lightMessage);
      return 1;
    }
    sim->lightPos[i] = lightCoord->valuedouble;
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

// creates the floor of the simulation
void floorInit(Simulation* sim)
{
  // create floor
  const float signs[4][2] = 
  {
    {-1.0f, -1.0f},
    {-1.0f, 1.0f},
    {1.0f, -1.0f},
    {1.0f, 1.0f}
  };
  const float side = 1000.0f;
  const unsigned int floatsPerVertex = 6;
  const unsigned int floatsPerTriangle = 3 * floatsPerVertex;
  vec3 normal = {0.0f, 1.0f, 0.0f};
  for(int i = 0; i < 2; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      int idx = i * floatsPerTriangle + j * floatsPerVertex;
      vec3 coord = {signs[i + j][0] * side, 0.0f, signs[i + j][1] * side};
      glm_vec3_copy(coord, sim->floorMesh + idx);
      glm_vec3_copy(normal, sim->floorMesh + idx + 3);
    }
  }

  glGenVertexArrays(1, &sim->floorVAO);
  glGenBuffers(1, &sim->floorVBO);
  glBindVertexArray(sim->floorVAO);
  glBindBuffer(GL_ARRAY_BUFFER, sim->floorVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sim->floorMesh), sim->floorMesh, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// initalizes and binds default meshes
int buffersInit(Simulation* sim)
{
  floorInit(sim);
  sim->meshSizes[SPHERE] = sphereIcoMeshSize();
  sim->meshSizes[CUBE] = cubeMeshSize();
  sim->meshSizes[TETRAHEDRON] = tetrahedronMeshSize();

  void (*generateMesh[OBJECT_TYPES])(float*); 
  generateMesh[SPHERE] = sphereIcoMesh;
  generateMesh[CUBE] = cubeMesh;
  generateMesh[TETRAHEDRON] = tetrahedronMesh;

  glGenVertexArrays(OBJECT_TYPES, sim->VAOs);
  glGenBuffers(OBJECT_TYPES, sim->VBOs);
  glGenBuffers(OBJECT_TYPES, sim->instanceVBOs);

  // bind each mesh to respective VAOs
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    sim->meshes[type] = malloc(sim->meshSizes[type] * sizeof(float));
    generateMesh[type](sim->meshes[type]);

    glBindVertexArray(sim->VAOs[type]);
    glBindBuffer(GL_ARRAY_BUFFER, sim->VBOs[type]);

    glBufferData(GL_ARRAY_BUFFER, sim->meshSizes[type] * sizeof(float), sim->meshes[type], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(6);

    sim->vertexCounts[type] = sim->objectCounts[type] * objectVerticesSize();
    sim->vertices[type] = malloc(sim->vertexCounts[type] * sizeof(float));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
  shaderInit(&sim->shader, "../src/render/shaders/default.vs", "../src/render/shaders/default.fs");
  shaderInit(&sim->floorShader, "../src/render/shaders/floor.vs", "../src/render/shaders/floor.fs");

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

void floorRender(Simulation* sim)
{
  shaderUse(&sim->floorShader);
  mat4 view, projection;
  cameraView(&sim->camera, view);
  cameraProjection(&sim->camera, projection);
  shaderSetMatrix(&sim->floorShader, "view", view);
  shaderSetMatrix(&sim->floorShader, "projection", projection);
  shaderSetVector(&sim->floorShader, "lightPos", sim->lightPos);

  glBindVertexArray(sim->floorVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void simulationRender(Simulation* sim)
{
  floorRender(sim);

  shaderUse(&sim->shader);

  mat4 view, projection;
  cameraView(&sim->camera, view);
  cameraProjection(&sim->camera, projection);
  shaderSetMatrix(&sim->shader, "view", view);
  shaderSetMatrix(&sim->shader, "projection", projection);

  shaderSetVector(&sim->shader, "lightPos", sim->lightPos);
  vec3 lightColor = {1.0f, 1.0f, 1.0f};
  shaderSetVector(&sim->shader, "lightColor", lightColor);
  shaderSetVector(&sim->shader, "viewPos", sim->camera.cameraPos);

  for(unsigned int type = 0; type < OBJECT_TYPES; type++)
  {
    glBindVertexArray(sim->VAOs[type]);

    glBindBuffer(GL_ARRAY_BUFFER, sim->instanceVBOs[type]);
    for(unsigned int i = 0, idx = 0; i < sim->objectCounts[type]; i++, idx += objectVerticesSize())
    {
      objectVertices(&sim->objects[type][i], sim->vertices[type] + idx);
    }
    glBufferData(GL_ARRAY_BUFFER, sim->vertexCounts[type] * sizeof(float), sim->vertices[type], GL_STATIC_DRAW);

    for(unsigned int i = 0; i < 4; i++)
    {
      glEnableVertexAttribArray(i + 1);
      glVertexAttribPointer(i + 1, 4, GL_FLOAT, GL_FALSE, objectVerticesSize() * sizeof(float), (void*) (i * 4 * sizeof(float)));
      glVertexAttribDivisor(i + 1, 1);
    }
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, objectVerticesSize() * sizeof(float), (void*) (16 * sizeof(float)));
    glVertexAttribDivisor(5, 1);

    glDrawArraysInstanced(GL_TRIANGLES, 0, sim->meshSizes[type] / 6, sim->objectCounts[type]);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
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
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // update particle positions
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - sim->lastTime;
    simulationUpdate(sim, sim->timeRatio * deltaTime);

    // calculate FPS
    printf("%f\n", 1000.0f / deltaTime);

    sim->lastTime = currentTime;

    simulationRender(sim);
    // simulationPrint(sim);

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

  printf("MESHES\n");
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    printf("%s %d\n", OBJECT_NAMES[type], sim->meshSizes[type]);
    for(int i = 0; i < sim->meshSizes[type]; i += 3)
    {
      printf("(%f, %f, %f)\n", sim->meshes[type][i], sim->meshes[type][i + 1], sim->meshes[type][i + 2]);
    }
    printf("\n");
  }

  // printf("VERTICES\n");
  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   printf("%s: %d\n", OBJECT_NAMES[type], sim->vertexCounts[type]);
  // }
  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   for(int i = 0; i < sim->objectCounts[type]; i++)
  //   {
  //     for(int j = 0; j < objectVerticesSize(); j++)
  //     {
  //       printf("%f ", sim->vertices[type][i * objectVerticesSize() + j]);
  //     }
  //     printf("\n");
  //   }
  //   printf("\n");
  // }

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

  printf("FLOOR\n");
  for(int i = 0; i < 36; i += 3)
  {
    printf("(%f, %f, %f)\n", sim->floorMesh[i], sim->floorMesh[i + 1], sim->floorMesh[i + 2]);
  }
}

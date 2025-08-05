#include "simulation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "physics/objects/floor.h"
#include "physics/objects/sphere.h"
#include "physics/objects/cube.h"
#include "physics/objects/tetrahedron.h"
#include "utils/parse.h"
#include "utils/save.h"

// initializes OpenGL and GLFW boilerplate
unsigned int openglInit(Simulation* sim)
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

  glfwWindowHint(GLFW_SAMPLES, 4);

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

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  return 0;
}

void buffersInit(Simulation* sim)
{
  sim->meshSizes[FLOOR] = floorMeshSize();
  sim->meshSizes[SPHERE] = sphereIcoMeshSize();
  sim->meshSizes[CUBE] = cubeMeshSize();
  sim->meshSizes[TETRAHEDRON] = tetrahedronMeshSize();

  void (*generateMesh[OBJECT_TYPES])(float*); 
  generateMesh[FLOOR] = floorMesh;
  generateMesh[SPHERE] = sphereIcoMesh;
  generateMesh[CUBE] = cubeMesh;
  generateMesh[TETRAHEDRON] = tetrahedronMesh;

  glGenVertexArrays(OBJECT_TYPES, sim->VAOs);
  glGenBuffers(OBJECT_TYPES, sim->VBOs);
  glGenBuffers(OBJECT_TYPES, sim->instanceVBOs);

  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    sim->meshes[type] = malloc(sim->meshSizes[type] * sizeof(float));
    generateMesh[type](sim->meshes[type]);

    sim->vertexCounts[type] = sim->objectCounts[type] * objectVerticesSize();
    sim->vertices[type] = malloc(sim->vertexCounts[type] * sizeof(float));


    glBindVertexArray(sim->VAOs[type]);

    // mesh vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, sim->VBOs[type]);
    glBufferData(GL_ARRAY_BUFFER, sim->meshSizes[type] * sizeof(float), sim->meshes[type], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(6);

    // instance vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, sim->instanceVBOs[type]);
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

    glBindVertexArray(0);
  }
}

unsigned int simulationInit(Simulation* sim, const char* configPath)
{
  // OpenGL boilerplate
  if(openglInit(sim))
  {
    return 1;
  }

  // initialize objects from config
  if(parseConfig(sim, configPath))
  {
    return 1;
  }

  sim->lastTime = 0.0f;
  sim->timeRatio = 0.5f;

  shaderInit(&sim->shader, "../src/render/shaders/default.vs", "../src/render/shaders/default.fs");
  cameraInit(&sim->camera, sim->window);
  if(shadowInit(&sim->shadow, &sim->camera, sim->lightDir))
  {
    return 1;
  }

  // initalize default meshes and bind
  buffersInit(sim);  

  return 0;
}

void simulationUpdate(Simulation* sim)
{
  if(glfwGetKey(sim->window, GLFW_KEY_ESCAPE))
  {
    glfwSetWindowShouldClose(sim->window, 1);
  }

  cameraKeyboardCallback(&sim->camera, sim->window);

  float currentTime = glfwGetTime();
  float deltaTime = currentTime - sim->lastTime;

  printf("%f\n", deltaTime);

  sim->lastTime = currentTime;

  cameraUpdate(&sim->camera);
  shadowUpdate(&sim->shadow, &sim->camera);
}

void objectsRender(Simulation* sim)
{
  for(unsigned int type = 0; type < OBJECT_TYPES; type++)
  {
    glBindVertexArray(sim->VAOs[type]);
    for(unsigned int i = 0, idx = 0; i < sim->objectCounts[type]; i++, idx += objectVerticesSize())
    {
      objectVertices(&sim->objects[type][i], sim->vertices[type] + idx);
    }

    glBindBuffer(GL_ARRAY_BUFFER, sim->instanceVBOs[type]);
    glBufferData(GL_ARRAY_BUFFER, sim->vertexCounts[type] * sizeof(float), sim->vertices[type], GL_STATIC_DRAW);
    glDrawArraysInstanced(GL_TRIANGLES, 0, sim->meshSizes[type] / 6, sim->objectCounts[type]);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

void simulationRender(Simulation* sim)
{
  // shadow pass
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  glViewport(0, 0, sim->shadow.SHADOW_WIDTH, sim->shadow.SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, sim->shadow.FBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  shaderUse(&sim->shadow.shader);
  shaderSetMatrix(&sim->shadow.shader, "vp", sim->shadow.vp);
  objectsRender(sim);
  if(glfwGetKey(sim->window, GLFW_KEY_P))
  {
    saveFramebuffer(sim->shadow.FBO, sim->shadow.SHADOW_WIDTH, sim->shadow.SHADOW_HEIGHT);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  // normal lighting pass
  shaderUse(&sim->shader);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  
  shaderSetMatrix(&sim->shader, "vp", sim->camera.vp);
  shaderSetMatrix(&sim->shader, "shadowVP", sim->shadow.vp);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, sim->shadow.depthMap);

  shaderSetInt(&sim->shader, "depthMap", 1);
  shaderSetVector(&sim->shader, "lightDir", sim->lightDir);
  shaderSetVector(&sim->shader, "lightColor", GLM_VEC3_ONE);
  shaderSetVector(&sim->shader, "viewPos", sim->camera.cameraPos);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  objectsRender(sim);
}

void simulationFree(Simulation* sim)
{
  for(int type = 0; type < OBJECT_TYPES; type++)
  {
    free(sim->meshes[type]);
    free(sim->vertices[type]);
  }

  glDeleteFramebuffers(1, &sim->shadow.FBO);
  glDeleteBuffers(3, sim->VBOs);
  glDeleteVertexArrays(3, sim->VAOs);
  glDeleteProgram(sim->shader.ID);
}

void simulationStart(Simulation* sim)
{
  while(!glfwWindowShouldClose(sim->window))
  {
    simulationUpdate(sim);
    simulationRender(sim);

    // simulationPrint(sim);

    glfwSwapBuffers(sim->window);
    glfwPollEvents();
  }

  glfwTerminate();
  simulationFree(sim);
}

void simulationPrint(Simulation* sim)
{
  // printf("OBJECTS\n");
  // printf("Object Counts\n");
  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   printf("%s: %d\n", OBJECT_NAMES[type], sim->objectCounts[type]);
  // }
  // printf("\n");

  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   printf("%s\n", OBJECT_NAMES[type]);

  //   for(int i = 0; i < sim->objectCounts[type]; i++)
  //   {
  //     objectPrint(sim->objects[type] + i);
  //     printf("\n");
  //   }
  //   printf("\n");
  // }

  // printf("MESHES\n");
  // for(int type = 0; type < OBJECT_TYPES; type++)
  // {
  //   if(type != SPHERE)
  //   {
  //     continue;
  //   }
  //   printf("%s %d\n", OBJECT_NAMES[type], sim->meshSizes[type]);
  //   for(int i = 0; i < sim->meshSizes[type]; i += 3)
  //   {
  //     printf("(%f, %f, %f)\n", sim->meshes[type][i], sim->meshes[type][i + 1], sim->meshes[type][i + 2]);
  //   }
  //   printf("\n");
  // }

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
  
  cameraPrint(&sim->camera);
  shadowPrint(&sim->shadow);
}

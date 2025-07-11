#include "simulation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void simulationInit(Simulation* sim, unsigned int n) {
  sim->WINDOW_HEIGHT = 600;
  sim->WINDOW_WIDTH = 800;
  sim->n = n;

  vec3 position = {0.0f, 0.0f, 0.0f};
  vec3 color = {1.0f, 1.0f, 1.0f};
  for(int i = 0; i < n; i++) {
    objectInit(&sim->objects[i], OBJECT_PARTICLE, 0.5f, 0.5f, position, color);
  }

  if (!glfwInit()) {
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  sim->window = glfwCreateWindow(sim->WINDOW_WIDTH, sim->WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
  if(!sim->window) {
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(sim->window);

  // load OpenGL function pointers
  if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    return;
  }

  shaderInit(&sim->s, "../src/render/shaders/simple.vs", "../src/render/shaders/simple.fs");
  shaderUse(&sim->s);
}

void simulationUpdate(Simulation* sim, float deltaTime) {
}

void simulationRender(Simulation* sim) {
  for(int i = 0; i < sim->n; i++) {
    Object* o = sim->objects + i;
    o->render(sim, o->position, o->orientation, o->color, o->size);
  }
}

void simulationStart(Simulation* sim) {
  while(!glfwWindowShouldClose(sim->window)) {
    if(glfwGetKey(sim->window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(sim->window, 1);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    // simulationUpdate(sim, 0.0f);
    simulationRender(sim);

    glfwSwapBuffers(sim->window);

    glfwPollEvents();
  }

  glfwTerminate();
}

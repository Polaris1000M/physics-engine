#include "simulation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void simulationInit(Simulation* sim, unsigned int n) {
  // OpenGL and GLFW boilerplate
  if (!glfwInit()) {
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  sim->WINDOW_HEIGHT = 600;
  sim->WINDOW_WIDTH = 800;
  sim->window = glfwCreateWindow(sim->WINDOW_WIDTH, sim->WINDOW_HEIGHT, "ParticleSimulator", NULL, NULL);
  if(!sim->window) {
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(sim->window);

  // load OpenGL function pointers
  if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    return;
  }

  sim->gravity = 9.8f;
  sim->lastTime = 0.0f;
  sim->timeRatio = 0.5f;

  // initialize objects in simulation
  vec3 position = {0.0f, 0.0f, -1.0f};
  vec3 color = {1.0f, 1.0f, 1.0f};
  sim->n = n;
  // for(int i = 0; i < n; i++) {
  //   position[0] = (float) i / (float) (n - 1) * 0.75f + 0.75f;
  //   objectInit(&sim->objects[i], OBJECT_PARTICLE, 0.1f, 0.5f, position, color);
  // }
  objectInit(&sim->objects[0], OBJECT_PARTICLE, 0.1f, 0.5f, (vec3) {-0.5f, 0.0f, 0.0f}, (vec3) {1.0f, 1.0f, 1.0f});
  objectInit(&sim->objects[1], OBJECT_PARTICLE, 0.1f, 0.5f, (vec3) {0.5f, 0.0f, 0.0f}, (vec3) {1.0f, 1.0f, 1.0f});

  // initialize shader programs
  shaderInit(&sim->s, "../src/render/shaders/simple.vs", "../src/render/shaders/simple.fs");

  shaderInit(&sim->particleShader, "../src/render/shaders/particle.vs", "../src/render/shaders/particle.fs");

  // initialize camera
  cameraInit(&sim->c, sim->window);
  glEnable(GL_DEPTH_TEST); // draws pixel if it is not behind another pixel
}

void simulationUpdate(Simulation* sim, float deltaTime) {
  for(int i = 0; i < sim->n; i++) {
    sim->objects[i].position[1] -= deltaTime;
  }
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

    cameraKeyboardCallback(&sim->c, sim->window);

    // printf("Pos: %f, %f, %f\n", sim->c.cameraPos[0], sim->c.cameraPos[1], sim->c.cameraPos[2]);
    // printf("Front: %f, %f, %f\n", sim->c.cameraFront[0], sim->c.cameraFront[1], sim->c.cameraFront[2]);
    // printf("Up: %f, %f, %f\n", sim->c.cameraUp[0], sim->c.cameraUp[1], sim->c.cameraUp[2]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update particle positions
    float currentTime = glfwGetTime();
    simulationUpdate(sim, sim->timeRatio * (currentTime - sim->lastTime));
    sim->lastTime = currentTime;

    simulationRender(sim);

    glfwSwapBuffers(sim->window);

    glfwPollEvents();
  }

  glfwTerminate();
}

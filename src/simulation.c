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
}

void simulationUpdate(Simulation* sim, float deltaTime) {
}

void simulationRenderObject(Object* o) {
  float vertices[] = {
    -0.5f, -0.5f, -1.0f,
    0.0f, 0.5f, -1.0f,
    0.5f, -0.5f, -1.0f
  };

  // buffer object to store vertices on GPU
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexPointer(3, GL_FLOAT, 0, NULL);
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawArrays(GL_TRIANGLES, 0, 1);
}

void simulationRender(Simulation* sim) {
  for(int i = 0; i < sim->n; i++) {
    simulationRenderObject(sim->objects + i);
  }
}

void simulationStart(Simulation* sim) {
  if (!glfwInit()) {
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow* window;


  window = glfwCreateWindow(sim->WINDOW_WIDTH, sim->WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window);

  // load OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    return;
  }

  Shader s;
  shaderInit(&s, "../src/render/shaders/simple.vs", "../src/render/shaders/simple.fs");
  shaderUse(&s);

  while (!glfwWindowShouldClose(window)) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, 1);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    simulationUpdate(sim, 0.0f);
    simulationRender(sim);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();
}

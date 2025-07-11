#include "object.h"
#include <stdlib.h>
#include <cglm/cglm.h>
#include "../simulation.h"

void renderParticle(Simulation* sim, vec3 position, vec3 orientation, vec3 color, float radius) {
  // equilateral triangle which minimally circumscribes circle has side length 2 * radius * 3^0.5

  float halfSideLength = radius * 1.73205081f;
  float vertices[] = {
    position[0] - halfSideLength, position[1] - radius, position[2],
    position[0], position[1] + 2.0f * radius, position[2],
    position[0] + halfSideLength, position[1] - radius, position[2]
  };

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);

  shaderSetFloat(&sim->particleShader, "aspectRatio", (float) sim->WINDOW_WIDTH / (float) sim->WINDOW_HEIGHT);
  shaderSetFloat(&sim->particleShader, "radiusSquared", radius * radius);
  shaderSetVector(&sim->particleShader, "center", position[0], position[1], position[2]);
  shaderUse(&sim->particleShader);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void objectInit(Object* o, ObjectType type, float size, float mass, vec3 position, vec3 color) {
  o->type = type;
  o->size = size;
  o->mass = mass;
  glm_vec3_copy(o->position, position);
  glm_vec3_copy(o->color, color);
  glm_vec3_copy(o->velocity, GLM_VEC3_ZERO);
  glm_vec3_copy(o->orientation, GLM_VEC3_ZERO);

  switch(type) {
    case OBJECT_PARTICLE:
      o->render = renderParticle;
      break;
    case OBJECT_CUBE:
      break;
    case OBJECT_TRIANGLE:
      break;
    default:
      return;
  }
}

#include "particle.h"
#include <glad/glad.h>
#include "../render/shader.h"
#include "../render/camera.h"

void renderParticle(Simulation* sim, vec3 position, vec3 orientation, vec3 color, float radius) {

  shaderSetVector(&sim->particleShader, "center", position);

  // equilateral triangle which minimally circumscribes circle has side length 2 * radius * 3^0.5
  float halfSideLength = radius * 1.73205081f;

  // create equilateral triangle around provided center point
  float vertices[] = {
    position[0] - halfSideLength, position[1] - radius, position[2],
    position[0], position[1] + 2.0f * radius, position[2],
    position[0] + halfSideLength, position[1] - radius, position[2]
  };

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  // store data in buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // indicate data layout
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  // set uniforms
  shaderSetFloat(&sim->particleShader, "radiusSquared", radius * radius);
  shaderSetVector(&sim->particleShader, "center", position);

  // update matrix uniforms
  mat4 model = GLM_MAT4_IDENTITY;
  shaderSetMatrix(&sim->particleShader, "model", model);

  // updates view to match where camera is currently pointing
  mat4 view;
  cameraLookAt(&sim->c, view);
  shaderSetMatrix(&sim->particleShader, "view", view);

  // creates perspective
  mat4 projection = GLM_MAT4_IDENTITY;
  glm_perspective(glm_rad(sim->c.fov), (float) sim->WINDOW_WIDTH / (float) sim->WINDOW_HEIGHT, 0.1f, 100.0f, projection);
  shaderSetMatrix(&sim->particleShader, "projection", projection);

  shaderUse(&sim->particleShader);

  // disable depth test for transparent portions of triangle
  glDepthMask(GL_FALSE);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glDepthMask(GL_TRUE);
}

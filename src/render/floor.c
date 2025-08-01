#include "floor.h"

void floorInit(Floor* f)
{
  shaderInit(&f->shader, "../src/render/shaders/floor.vs", "../src/render/shaders/floor.fs");

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
      glm_vec3_copy(coord, f->mesh+ idx);
      glm_vec3_copy(normal, f->mesh+ idx + 3);
    }
  }

  glGenVertexArrays(1, &f->VAO);
  glGenBuffers(1, &f->VBO);
  glBindVertexArray(f->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, f->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(f->mesh), f->mesh, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void floorRender(Floor* f, Camera* c, vec3 lightPos)
{
  shaderUse(&f->shader);
  mat4 view, projection;
  cameraView(c, view);
  cameraProjection(c, projection);
  shaderSetMatrix(&f->shader, "view", view);
  shaderSetMatrix(&f->shader, "projection", projection);
  shaderSetVector(&f->shader, "lightPos", lightPos);

  glBindVertexArray(f->VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

#include "sphere.h"
#include <math.h>
#include <stdlib.h>

void sphereInit(Sphere* s, float radius, unsigned int sectors, unsigned int stacks) {
  s->radius = radius;
  s->sectors = sectors;
  s->stacks = stacks;

  // sectors * stacks total rectangles
  // 2 triangles per rectangle
  // 9 coordinates per triangle
  // 9 colors per triangle
  s->n = 36 * sectors * stacks;
}

float* sphereVertices(Sphere* s, vec3 position) {
  float* vertices = malloc(sizeof(float) * s->n);

  float deltaStack = M_PI / (float) s->stacks;
  float deltaSector = M_PI * 2.0f / (float) s->sectors;

  for(unsigned int stack = 0; stack < s->stacks; stack++)
  {
    for(unsigned int sector = 0; sector < s->sectors; sector++)
    {
      float stackAngles[2];
      stackAngles[0] = M_PI / 2.0f - (float) stack * deltaStack;
      stackAngles[1] = stackAngles[0] - deltaStack;

      float sectorAngles[2];
      sectorAngles[0] = (float) sector * deltaSector;
      sectorAngles[1] = sectorAngles[0] + deltaSector;


      float x[4], y[4], z[4];
      for(unsigned int i = 0; i < 2; i++)
      {
        for(unsigned int j = 0; j < 2; j++)
        {
          int idx = i * 2 + j;
          x[idx] = position[0] + s->radius * cos(stackAngles[i]) * sin(sectorAngles[j]);
          y[idx] = position[1] + s->radius * sin(stackAngles[i]);
          z[idx] = position[2] + s->radius * cos(stackAngles[i]) * cos(sectorAngles[j]);
        }
      }
      
      int floatsPerTriangle = 18;
      int floatsPerVertex = 6;
      int idx = stack * s->sectors * floatsPerTriangle * 2 + sector * floatsPerTriangle * 2;

      // iterate over triangles
      for(int i = 0; i < 2; i++)
      {
        // iterate over vertices
        for(int j = 0; j < 3; j++)
        {
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex] = x[i + j];
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 1] = y[i + j];
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 2] = z[i + j];
        }
      }

      // set color
      for(int i = 0; i < 2; i++)
      {
        int color = (stack * s->sectors + sector) % 3;
        for(int j = 0; j < 3; j++)
        {
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 3] = 0;
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 4] = 0;
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 5] = 0;

          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 3 + color] = 1;
        }
      }

      if(stack == 0) {
        for(int j = 0; j < 3; j++)
        {
          vertices[idx + j * floatsPerVertex] = x[1 + j];
          vertices[idx + j * floatsPerVertex + 1] = y[1 + j];
          vertices[idx + j * floatsPerVertex + 2] = z[1 + j];
        }
      }
      else if(stack == s->stacks - 1) {
        for(int j = 0; j < 3; j++)
        {
          vertices[idx + j * floatsPerVertex] = x[j];
          vertices[idx + j * floatsPerVertex + 1] = y[j];
          vertices[idx + j * floatsPerVertex + 2] = z[j];
        }
      }
    }
  }

  return vertices;
}

void renderSphere(Simulation* sim, vec3 position, vec3 orientation, vec3 color, float radius)
{
  Sphere s;
  sphereInit(&s, radius, 10, 10);

  float* vertices = sphereVertices(&s, position);

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  // store data in buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s.n, vertices, GL_STATIC_DRAW);

  // indicate data layout
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // set uniforms
  shaderSetFloat(&sim->s, "radiusSquared", radius * radius);
  shaderSetVector(&sim->s, "center", position);

  // update matrix uniforms
  mat4 model = GLM_MAT4_IDENTITY;
  shaderSetMatrix(&sim->s, "model", model);

  // updates view to match where camera is currently pointing
  mat4 view;
  cameraLookAt(&sim->c, view);
  shaderSetMatrix(&sim->s, "view", view);

  // creates perspective
  mat4 projection = GLM_MAT4_IDENTITY;
  glm_perspective(glm_rad(sim->c.fov), (float) sim->WINDOW_WIDTH / (float) sim->WINDOW_HEIGHT, 0.1f, 100.0f, projection);
  shaderSetMatrix(&sim->s, "projection", projection);

  shaderUse(&sim->s);

  glDrawArrays(GL_TRIANGLES, 0, 6 * s.stacks * s.sectors);

  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

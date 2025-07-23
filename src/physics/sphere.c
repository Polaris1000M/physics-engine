#include "sphere.h"
#include <math.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <string.h>
#include <cglm/cglm.h>
#include <stdio.h>

#define STACKS 10 // number of stacks in a sphere
#define SECTORS 10 // number of sectors in a sphere
#define RECUR 4 // number of recursive levels for a sphere

unsigned int count;

void sphereUVMesh(float* vertices)
{
  const float deltaStack = M_PI / (float) STACKS;
  const float deltaSector = M_PI * 2.0f / (float) SECTORS;
  const float defaultSize = 0.5f;

  for(unsigned int stack = 0; stack < STACKS; stack++)
  {
    float stackAngles[2];
    stackAngles[0] = M_PI / 2.0f - (float) stack * deltaStack;
    stackAngles[1] = stackAngles[0] - deltaStack;

    for(unsigned int sector = 0; sector < SECTORS; sector++)
    {
      float sectorAngles[2];
      sectorAngles[0] = (float) sector * deltaSector;
      sectorAngles[1] = sectorAngles[0] + deltaSector;

      float x[4], y[4], z[4];
      for(unsigned int i = 0; i < 2; i++)
      {
        for(unsigned int j = 0; j < 2; j++)
        {
          unsigned int idx = i * 2 + j;
          x[idx] = defaultSize * cos(stackAngles[i]) * sin(sectorAngles[j]);
          y[idx] = defaultSize * sin(stackAngles[i]);
          z[idx] = defaultSize * cos(stackAngles[i]) * cos(sectorAngles[j]);
        }
      }

      int floatsPerTriangle = 9;
      int floatsPerVertex = 3;
      int idx = stack * SECTORS * floatsPerTriangle * 2 + sector * floatsPerTriangle * 2;

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

      // edge case for top and bottom stacks
      if(stack == 0)
      {
        for(int j = 0; j < 3; j++)
        {
          vertices[idx + j * floatsPerVertex] = x[1 + j];
          vertices[idx + j * floatsPerVertex + 1] = y[1 + j];
          vertices[idx + j * floatsPerVertex + 2] = z[1 + j];
        }
      }
      else if(stack == STACKS - 1)
      {
        idx += floatsPerTriangle;
        for(int j = 0; j < 3; j++)
        {
          vertices[idx + j * floatsPerVertex] = x[j];
          vertices[idx + j * floatsPerVertex + 1] = y[j];
          vertices[idx + j * floatsPerVertex + 2] = z[j];
        }
      }
    }
  }
}

unsigned int sphereUVMeshSize()
{
  // stacks * sectors rectangles in total
  // 2 triangles per rectangle
  // 9 coordinates per triangle
  // 9 color coordinates per triangle
  return STACKS * SECTORS * 18;
}

typedef struct IcosphereFace IcosphereFace;

struct IcosphereFace
{
  vec3 vertices[3];
  IcosphereFace* next[4];
};

void populateIcoMesh(float* vertices, unsigned int* idx, IcosphereFace* face)
{
  if(face->next[0])
  {
    for(int i = 0; i < 4; i++)
    {
      populateIcoMesh(vertices, idx, face->next[i]);
    }

    return;
  }

  for(int i = 0; i < 3; i++)
  {
    // printf("(%f, %f, %f)\n", face->vertices[i][0], face->vertices[i][1], face->vertices[i][2]);

    glm_vec3_copy(face->vertices[i], vertices + *idx);
    *idx += 3;

    vertices[*idx] = count;
    *idx += 1;
  }

  count++;
}

void subdivideIcoFace(IcosphereFace* face, float defaultSize)
{
  if(face->next[0])
  {
    for(int i = 0; i < 4; i++)
    {
      subdivideIcoFace(face->next[i], defaultSize);
    }

    return;
  }

  for(int i = 0; i < 4; i++)
  {
    face->next[i] = malloc(sizeof(IcosphereFace));
  }

  for(int i = 0; i < 4; i++)
  {
    IcosphereFace* nextFace = face->next[i];
    for(int j = 0; j < 4; j++)
    {
      nextFace->next[j] = NULL;
    }
  }

  float midpoints[3][3];
  glm_vec3_add(face->vertices[0], face->vertices[1], midpoints[0]);
  glm_vec3_add(face->vertices[0], face->vertices[2], midpoints[1]);
  glm_vec3_add(face->vertices[1], face->vertices[2], midpoints[2]);

  for(int i = 0; i < 3; i++)
  {
    glm_vec3_scale_as(midpoints[i], defaultSize, midpoints[i]);
  }

  glm_vec3_copy(face->vertices[0], face->next[0]->vertices[0]);
  glm_vec3_copy(midpoints[0], face->next[0]->vertices[1]);
  glm_vec3_copy(midpoints[1], face->next[0]->vertices[2]);

  glm_vec3_copy(midpoints[0], face->next[1]->vertices[0]);
  glm_vec3_copy(face->vertices[1], face->next[1]->vertices[1]);
  glm_vec3_copy(midpoints[2], face->next[1]->vertices[2]);

  glm_vec3_copy(midpoints[1], face->next[2]->vertices[0]);
  glm_vec3_copy(midpoints[2], face->next[2]->vertices[1]);
  glm_vec3_copy(face->vertices[2], face->next[2]->vertices[2]);

  glm_vec3_copy(midpoints[2], face->next[3]->vertices[0]);
  glm_vec3_copy(midpoints[1], face->next[3]->vertices[1]);
  glm_vec3_copy(midpoints[0], face->next[3]->vertices[2]);
}

void sphereIcoMesh(float* vertices)
{
  // generate icosphere circumscribed in sphere of radius 0.5

  float goldenRatio = 1.61803398875f;
  float radiusToSideRatio = sqrt(goldenRatio * goldenRatio + 1.0f) / 2.0f;
  radiusToSideRatio = 1.0f / radiusToSideRatio;
  const float defaultSize = 0.5f;
  float sideLength = radiusToSideRatio * defaultSize;

  float topBase[5][3], bottomBase[5][3];
  topBase[0][0] = 0.0f;
  topBase[0][1] = sideLength * 0.5f;
  topBase[0][2] = sqrt(0.5f * 0.5f - topBase[0][1] * topBase[0][1]);

  mat4 rot = GLM_MAT4_IDENTITY;
  glm_rotate_y(rot, glm_rad(72.0f), rot);
  for(int i = 1; i < 5; i++)
  {
    glm_mat4_mulv3(rot, topBase[i - 1], 1.0f, topBase[i]);
  }

  glm_mat4_identity(rot);
  glm_rotate_y(rot, glm_rad(36.0f), rot);
  for(int i = 0; i < 5; i++)
  {
    glm_mat4_mulv3(rot, topBase[i], 1.0f, bottomBase[i]);
    bottomBase[i][1] = -topBase[i][1];
  }

  IcosphereFace icosphere[20];
  for(int i = 0; i < 20; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      icosphere[i].next[j] = NULL;
    }
  }

  // populate upper pentagonal pyramid
  float top[3] = {0.0f, defaultSize, 0.0f};
  for(int i = 0; i < 5; i++)
  {
    glm_vec3_copy(top, icosphere[i].vertices[0]);
    glm_vec3_copy(topBase[i], icosphere[i].vertices[1]);
    glm_vec3_copy(topBase[(i + 1) % 5], icosphere[i].vertices[2]);
  }

  // populate lower pentagonal pyramid
  float bottom[3] = {0.0f, -defaultSize, 0.0f};
  for(int i = 0; i < 5; i++)
  {
    glm_vec3_copy(bottom, icosphere[i + 5].vertices[0]);
    glm_vec3_copy(bottomBase[(i + 1) % 5], icosphere[i + 5].vertices[1]);
    glm_vec3_copy(bottomBase[i], icosphere[i + 5].vertices[2]);
  }

  // populate middle faces
  unsigned int idx = 10;
  for(int i = 0; i < 5; i++)
  {
    glm_vec3_copy(topBase[i], icosphere[idx].vertices[0]);
    glm_vec3_copy(bottomBase[(i + 4) % 5], icosphere[idx].vertices[1]);
    glm_vec3_copy(bottomBase[i], icosphere[idx].vertices[2]);
    idx++;

    glm_vec3_copy(bottomBase[i], icosphere[idx].vertices[0]);
    glm_vec3_copy(topBase[(i + 1) % 5], icosphere[idx].vertices[1]);
    glm_vec3_copy(topBase[i], icosphere[idx].vertices[2]);
    idx++;
  }

  for(int i = 0; i < RECUR; i++)
  {
    for(int j = 0; j < 20; j++)
    {
      subdivideIcoFace(icosphere + j, defaultSize);
    }
  }

  idx = 0;
  for(int i = 0; i < 20; i++)
  {
    populateIcoMesh(vertices, &idx, icosphere + i);
  }
  printf("%d\n", idx);
}

unsigned int sphereIcoMeshSize()
{
  // original icosahedron has 20 triangles
  unsigned int result = 20;

  // each subdivision results in four times more triangles
  for(int i = 0; i < RECUR; i++)
  {
    result *= 4;
  }

  // each triangle has 3 vertices and 3 floats per vertex
  result *= 3 * 4;

  return result;
}

#include "sphere.h"
#include <math.h>
#include <stdlib.h>

#define STACKS 10 // number of stacks in a sphere
#define SECTORS 10 // number of sectors in a sphere

void sphereVertices(float* vertices, Object* s)
{
  float deltaStack = M_PI / (float) STACKS;
  float deltaSector = M_PI * 2.0f / (float) SECTORS;

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
          x[idx] = s->size * cos(stackAngles[i]) * sin(sectorAngles[j]);
          y[idx] = s->size * sin(stackAngles[i]);
          z[idx] = s->size* cos(stackAngles[i]) * cos(sectorAngles[j]);
        }
      }
      
      int floatsPerTriangle = 18;
      int floatsPerVertex = 6;
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

      // set color
      for(int i = 0; i < 2; i++)
      {
        int color = (stack * SECTORS + sector) % 3;
        for(int j = 0; j < 3; j++)
        {
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 3] = 0;
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 4] = 0;
          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 5] = 0;

          vertices[idx + i * floatsPerTriangle + j * floatsPerVertex + 3 + color] = 1;
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


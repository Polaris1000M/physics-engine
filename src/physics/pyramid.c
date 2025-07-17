#include "pyramid.h"
#include <math.h>
#include <cglm/cglm.h>
#include <string.h>

void pyramidMesh(float* vertices)
{
  float coords[4][3];

  const float defaultSize = 0.5f;
  const float angleDown = 0.339836909454f;
  coords[0][0] = defaultSize * cos(angleDown);
  coords[0][1] = defaultSize * -sin(angleDown);
  coords[0][2] = 0.0f;

  // create other points on the base of the pyramid by rotating prior point 120 degrees about the y axis
  mat4 rotate = GLM_MAT4_IDENTITY;
  glm_rotate_y(rotate, 2.0f * M_PI / 3.0f, rotate);
  for(int i = 1; i < 3; i++)
  {
    glm_mat4_mulv3(rotate, coords[i - 1], 1.0f, coords[i]);
  }

  coords[3][0] = 0.0f;
  coords[3][1] = defaultSize;
  coords[3][2] = 0.0f;

  const unsigned int floatsPerTriangle = 9;
  const unsigned int floatsPerVertex = 3;
  for(int removed = 0; removed < 4; removed++)
  {
    int ct = 0;
    for(int coord = 0; coord < 4; coord++)
    {
      if(coord == removed)
      {
        continue;
      }

      int idx = removed * floatsPerTriangle + ct * floatsPerVertex;
      ct++;

      memcpy(vertices + idx, coords[coord], 3 * sizeof(float));
    }
  }
}

unsigned int pyramidMeshSize()
{
  // 4 triangles
  // 9 coordinates per triangle
  return 36;
}

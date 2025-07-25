#include "cube.h"
#include <string.h>

void cubeMesh(float* vertices)
{
  const float defaultSize = 1.0f;

  // half of a side length of the square
  // all points in the cube have coordinates either positive or negative half
  // multiplied by square root of 3
  const float half = defaultSize / 1.73205081f;
  
  const float sign[2] = {-1.0f, 1.0f};
  const int floatsPerVertex = 6;
  const int floatsPerTriangle = 3 * floatsPerVertex;
  const float signs[4][2] =
  {
    {-1.0f, 1.0f},
    {1.0f, 1.0f},
    {-1.0f, -1.0f},
    {1.0f, -1.0f}
  };
  
  // all faces in cube are defined by one fixed coordinate which has the same value for all coordinates
  // fixed is the axis which is fixed
  for(int fixed = 0; fixed < 3; fixed++)
  {
    // the sign of the fixed value
    for(int fixedSign = 0; fixedSign < 2; fixedSign++)
    {
      int idx = fixed * 2 * floatsPerTriangle;

      float face[4][3];
      for(int sign = 0; sign < 4; sign++)
      {
        face[sign][fixed] = half * (2 * fixedSign - 1);
        face[sign][(fixed + 1) % 3] = signs[sign][0] * half;
        face[sign][(fixed + 2) % 3] = signs[sign][1] * half;
      }

      vec3 normal = {0.0f, 0.0f, 0.0f};
      normal[fixed] = sign[fixedSign];
      for(int i = 0; i < 2; i++)
      {
        for(int j = 0; j < 3; j++)
        {
          int idx = fixed * 4 * floatsPerTriangle + fixedSign * 2 * floatsPerTriangle + i * floatsPerTriangle + j * floatsPerVertex;
          glm_vec3_copy(face[i + j], vertices + idx);
          glm_vec3_copy(normal, vertices + idx + 3);
        }
      }
    }
  }
}

unsigned int cubeMeshSize()
{
  // 6 faces
  // 2 triangles per face
  // 18 coordinates triangle
  return 216;
}

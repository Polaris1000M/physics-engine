#include "sphere.h"
#include <math.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <string.h>
#include <cglm/cglm.h>

#define STACKS 10 // number of stacks in a sphere
#define SECTORS 10 // number of sectors in a sphere
#define RECUR 5 // number of recursive levels for a sphere

// represents a vertex in the icosphere 
// all indices represent indices into central parent icosphere vertex array
typedef struct IcoVertex
{
  unsigned int neighbors[6]; // the six adjacent vertices to the current one
  float normal[3];           // normal vector of the current vertex
  float coords[3];           // coordinates of this vertex
} IcoVertex;

void icoVertexPrint(IcoVertex *vertex)
{
  printf("(%f, %f, %f)\n", vertex->coords[0], vertex->coords[1], vertex->coords[2]);
  printf("(");
  for(int neighbor = 0; neighbor < 6; neighbor++)
  {
    printf("%d", vertex->neighbors[neighbor]);
    if(neighbor < 5)
    {
      printf(", ");
    }
  }
  printf(")\n\n");
}

// returns index of the target neighbor in the provided vertex
// returns -1 if the index is not found
int icoVertexFind(IcoVertex* vertex, int target)
{
  for(int i = 0; i < 6; i++)
  {
    if(vertex->neighbors[i] == target)
    {
      return i;
    }
  }

  return -1;
}

// replaces a default -1 value with the target value
// does nothing if the target already exists in the vertex or there are no more default values
void icoVertexAdd(IcoVertex* vertex, int neighbor)
{
  if(icoVertexFind(vertex, neighbor) != -1)
  {
    return;
  }

  for(int i = 0; i < 6; i++)
  {
    if(vertex->neighbors[i] == -1)
    {
      vertex->neighbors[i] = neighbor;
      return;
    }
  }
}

// replaces one of the neighbors of an icosphere vertex
// used in subdivision algorithm
void icoVertexReplace(IcoVertex* vertex, int cur, int next)
{
  for(int i = 0; i < 6; i++)
  {
    // also replace vertex if the index is still the default -1 value
    if(vertex->neighbors[i] == cur)
    {
      vertex->neighbors[i] = next;
      return;
    }
  }
}

// populate default icosphere vertex data
void icoVertexDefault(IcoVertex* vertex)
{
  memset(vertex->neighbors, -1, sizeof(vertex->neighbors));
  memset(vertex->normal, 0, sizeof(vertex->normal));
}

// checks if a midpoint already exists between the two provided points
int icoVertexMidpoint(IcoVertex* icoVertices, unsigned int vertexOne, unsigned int vertexTwo, vec3 target)
{
  const float EPS = 0.000000001f;
  for(int i = 0; i < 6; i++)
  {
    if(icoVertices[vertexOne].neighbors[i] == -1)
    {
      continue;
    }

    for(int j = 0; j < 6; j++)
    {
      if(icoVertices[vertexOne].neighbors[i] == icoVertices[vertexTwo].neighbors[j])
      {
        vec3 diff;
        unsigned int cur = icoVertices[vertexOne].neighbors[i];
        glm_vec3_sub(icoVertices[cur].coords, target, diff);
        if(glm_vec3_dot(diff, diff) < EPS)
        {
          return cur;
        }
      }
    }
  }

  return -1;
}
 
// represents a triangular face of the icosphere 
typedef struct IcoFace IcoFace;
struct IcoFace
{
  unsigned int indices[3]; // indices of the 3 vertices of the triangle
  IcoFace* next[4];        // next 4 triangles created from a subdivision of the current triangle
};

void icoFacePrint(IcoFace* face)
{
  printf("(%d, %d, %d)\n\n", face->indices[0], face->indices[1], face->indices[2]);
}

// populate default icosphere face data
void icoFaceDefault(IcoFace* face)
{
  memset(face->indices, -1, sizeof(face->indices));
  memset(face->next, 0, sizeof(face->next));
}

// creates edges between all the vertices in an icosphere face
void icoFaceConnect(IcoFace* face, IcoVertex* vertices)
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      icoVertexAdd(vertices + face->indices[i], face->indices[j]);
      icoVertexAdd(vertices + face->indices[j], face->indices[i]);
    }
  }
}

// the vertices and indices to propagate to child faces during subdivision process
// 0 indicates one of the parent's vertices while 1 indicates one of the new midpoints
// vertices and midpoints are all assumed to be in counter clockwise order
static const unsigned int divisionPattern[4][3][2] = 
{
  {
    {0, 0},
    {1, 0},
    {1, 1}
  },
  {
    {1, 0},
    {0, 1},
    {1, 2}
  },
  {
    {1, 1},
    {1, 2},
    {0, 2}
  },
  {
    {1, 2},
    {1, 1},
    {1, 0}
  }
};

// indicates which of the parent vertex indices the current midpoint lies between
static const unsigned int midpointPattern[3][2] = 
{
  {0, 1},
  {0, 2},
  {1, 2}
};

void icoFaceSubdivide(IcoFace* face, IcoVertex* icoVertices, float defaultSize, unsigned int* vertexIdx)
{
  // continual traversing tree if current face already subdivided
  if(face->next[0])
  {
    for(int i = 0; i < 4; i++)
    {
      icoFaceSubdivide(face->next[i], icoVertices, defaultSize, vertexIdx);
    }

    return;
  }

  // create new midpoint vertices if needed
  unsigned int midpointIndices[3];
  for(int midpoint = 0; midpoint < 3; midpoint++)
  {
    unsigned int idxOne = face->indices[midpointPattern[midpoint][0]];
    unsigned int idxTwo = face->indices[midpointPattern[midpoint][1]];

    vec3 target;
    glm_vec3_add(icoVertices[idxOne].coords, icoVertices[idxTwo].coords, target);
    glm_vec3_scale_as(target, defaultSize, target);
    midpointIndices[midpoint] = icoVertexMidpoint(icoVertices, idxOne, idxTwo, target);
    if(midpointIndices[midpoint] == -1)
    {
      // printf("%d %d %d\n", *vertexIdx, idxOne, idxTwo);
      // printf("(%f, %f, %f)\n\n", target[0], target[1], target[2]);
      midpointIndices[midpoint] = *vertexIdx;
      icoVertexReplace(icoVertices + idxOne, idxTwo, *vertexIdx);
      icoVertexReplace(icoVertices + idxTwo, idxOne, *vertexIdx);
      glm_vec3_copy(target, icoVertices[*vertexIdx].coords);
      (*vertexIdx)++;
    }
  }

  // create the new subfaces
  for(int subFace = 0; subFace < 4; subFace++)
  {
    face->next[subFace] = malloc(sizeof(IcoFace));
    icoFaceDefault(face->next[subFace]);
    IcoFace* nextFace = face->next[subFace];

    // update the indices of the new subface
    for(int vertex = 0; vertex < 3; vertex++)
    {
      unsigned int loc = divisionPattern[subFace][vertex][0];
      unsigned int idx = divisionPattern[subFace][vertex][1];

      if(loc == 0) // parent's vertex
      {
        nextFace->indices[vertex] = face->indices[idx];
      }
      else // new midpoint vertex
      {
        nextFace->indices[vertex] = midpointIndices[idx];
      }
    }

    icoFaceConnect(nextFace, icoVertices);
  }
}

void icoFaceComputeNormals(IcoFace* face, IcoVertex* icoVertices)
{
  if(face->next[0])
  {
    for(int i = 0; i < 4; i++)
    {
      icoFaceComputeNormals(face->next[i], icoVertices);
    }

    return;
  }

  vec3 v1, v2, normal;
  glm_vec3_sub(icoVertices[face->indices[1]].coords, icoVertices[face->indices[0]].coords, v1);
  glm_vec3_sub(icoVertices[face->indices[2]].coords, icoVertices[face->indices[0]].coords, v2);
  glm_vec3_cross(v1, v2, normal);
  for(int i = 0; i < 3; i++)
  {
    glm_vec3_add(icoVertices[face->indices[i]].normal, normal, icoVertices[face->indices[i]].normal);
  }
}

void icoFacePopulate(IcoFace* face, IcoVertex* icoVertices, float* vertices, unsigned int* faceIdx)
{
  if(face->next[0])
  {
    for(int i = 0; i < 4; i++)
    {
      icoFacePopulate(face->next[i], icoVertices, vertices, faceIdx);
    }

    return;
  }

  for(int i = 0; i < 3; i++)
  {
    glm_vec3_copy(icoVertices[face->indices[i]].coords, vertices + *faceIdx);
    glm_vec3_copy(icoVertices[face->indices[i]].normal, vertices + *faceIdx + 3);
    (*faceIdx) += 6;
  }
}

// the number of vertices in the finished icosphere
unsigned int icoVertexCount()
{
  unsigned int result = 12;

  // each subdivision produces no more than 3 times as many vertices
  // 6 new vertices for each vertex, but each new vertex is counted twice
  for(int i = 0; i < RECUR; i++)
  {
    result *= 5;
  }

  return result;
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

  // each triangle has 3 vertices and 6 floats per vertex
  result *= 3 * 6;

  return result;
}

void sphereIcoMesh(float* vertices)
{
  // generate icosphere circumscribed in sphere of default radius
  const float goldenRatio = 1.61803398875f;
  const float radiusToSideRatio = 1.0f / (sqrt(goldenRatio * goldenRatio + 1.0f) / 2.0f);
  const float defaultSize = 0.5f;
  const float sideLength = radiusToSideRatio * defaultSize;

  // initial icosahedron layout
  // 0 = top
  // 1-5 = top base
  // 6 = bottom
  // 7-11 = bottom base
  IcoVertex* icoVertices = malloc(icoVertexCount() * sizeof(IcoVertex));
  for(int i = 0; i < icoVertexCount(); i++)
  {
    icoVertexDefault(icoVertices + i);
  }

  // upper pentagonal base
  icoVertices[0].coords[0] = 0.0f;
  icoVertices[0].coords[1] = defaultSize;
  icoVertices[0].coords[2] = 0.0f;
  icoVertices[1].coords[0] = 0.0f;
  icoVertices[1].coords[1] = sideLength * 0.5f;
  icoVertices[1].coords[2] = sqrt(0.5f * 0.5f - (sideLength * 0.5f) * (sideLength * 0.5f));
  mat4 rot = GLM_MAT4_IDENTITY;
  glm_rotate_y(rot, glm_rad(72.0f), rot);
  for(int i = 2; i <= 5; i++)
  {
    glm_mat4_mulv3(rot, icoVertices[i - 1].coords, 1.0f, icoVertices[i].coords);
  }
  
  // lower pentagonal base
  icoVertices[6].coords[0] = 0.0f;
  icoVertices[6].coords[1] = -defaultSize;
  icoVertices[6].coords[2] = 0.0f;
  glm_mat4_identity(rot);
  glm_rotate_y(rot, glm_rad(36.0f), rot);
  for(int i = 7; i <= 11; i++)
  {
    glm_mat4_mulv3(rot, icoVertices[i - 6].coords, 1.0f, icoVertices[i].coords);
    icoVertices[i].coords[1] = -sideLength * 0.5f;
  }

  // populate default array of icosphere faces
  IcoFace icoFaces[20];
  for(int i = 0; i < 20; i++)
  {
    icoFaceDefault(icoFaces + i);
  }

  // populate initial icosahedron data
  unsigned int topOffset = 1;
  unsigned int bottomOffset = 7;
  for(int face = 0; face < 5; face++)
  {
    // upper pentagonal pyramid
    const unsigned int curTopIndices[3] = {0, face + topOffset, (face + 1) % 5 + topOffset};

    // lower pentagonal pyramid
    const unsigned int curBottomIndices[3] = {6, (face + 1) % 5 + bottomOffset, face + bottomOffset};

    // right side up middle faces
    const unsigned int curUpIndices[3] = {face + topOffset, (face + 4) % 5 + bottomOffset, face + bottomOffset};

    // upside down middle faces
    const unsigned int curDownIndices[3] = {face + bottomOffset, (face + 1) % 5 + topOffset, face + topOffset};

    for(int vertex = 0; vertex < 3; vertex++)
    {
      // update current face to include index
      icoFaces[face].indices[vertex] = curTopIndices[vertex];
      icoFaces[face + 5].indices[vertex] = curBottomIndices[vertex];
      icoFaces[face + 10].indices[vertex] = curUpIndices[vertex];
      icoFaces[face + 15].indices[vertex] = curDownIndices[vertex];
    }

    icoFaceConnect(icoFaces + face, icoVertices);
    icoFaceConnect(icoFaces + face + 5, icoVertices);
    icoFaceConnect(icoFaces + face + 10, icoVertices);
    icoFaceConnect(icoFaces + face + 15, icoVertices);
  }

  // begin subdivisions
  unsigned int vertexIdx = 12; // the index the next faces will take during subdivision
  for(int i = 0; i < RECUR; i++)
  {
    for(int j = 0; j < 20; j++)
    {
      icoFaceSubdivide(icoFaces + j, icoVertices, defaultSize, &vertexIdx);
    }
  }

  // compute normals
  for(int i = 0; i < 20; i++)
  {
    icoFaceComputeNormals(icoFaces + i, icoVertices);
  }
  for(int i = 0; i < vertexIdx; i++)
  {
    glm_vec3_scale_as(icoVertices[i].coords, defaultSize, icoVertices[i].coords);
    glm_vec3_normalize(icoVertices[i].normal);
  }

  // populate resulting data into vertex buffer
  unsigned int faceIdx = 0; // the index the current face will have in the vertex buffer
  for(int i = 0; i < 20; i++)
  {
    icoFacePopulate(icoFaces + i, icoVertices, vertices, &faceIdx);
  }
}

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


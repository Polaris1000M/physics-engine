#include "sphere.h"
#include <math.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <string.h>
#include <cglm/cglm.h>

#define STACKS 10 // number of stacks in a sphere
#define SECTORS 10 // number of sectors in a sphere
#define RECUR 1 // number of recursive levels for a sphere

// the initial count of vertices
static unsigned int icoVertexCount = 20;

// represents a triangular face of the icosphere 
typedef struct IcoFace IcoFace;
struct IcoFace
{
  unsigned int indices[3]; // indices of the 3 vertices of the triangle
  IcoFace* next[4];        // next 4 triangles created from a subdivision of the current triangle
};

// represents a vertex in the icosphere 
// all indices represent indices into central parent icosphere vertex array
typedef struct IcoVertex
{
  unsigned int neighbors[6]; // the six adjacent vertices to the current one
  float normal[3];           // normal vector of the current vertex
  float coords[3];           // coordinates of this vertex
} IcoVertex;

void icoFacePrint(IcoFace* face)
{
  printf("(%d, %d, %d)\n\n", face->indices[0], face->indices[1], face->indices[2]);
}

void icoVertexPrint(IcoVertex *vertex)
{
  printf("(%d, %d, %d)\n\n", vertex->coords[0], face->coords[1], face->coords[2]);
  printf("(");
  for(int neighbor = 0; neighbor < 6; neighbor++)
  {
    printf("%d", vertex->neighbors[neighbor]);
    if(neighbor < 5)
    {
      printf(", ");
    }
  }
  printf(")\n");
}

// returns index of the target neighbor in the provided vertex
// returns -1 if the index is not found
int icoVertexFindNeighbor(IcoVertex* vertex, int target)
{
  icoVertexPrint(vertex);
  for(int i = 0; i < 6; i++)
  {
    if(vertex->neighbors[i] == target)
    {
      return i;
    }
  }

  return -1;
}

// replaces the target neighbor in the vertex with -1
void icoVertexRemove(IcoVertex* vertex, int target)
{
  for(int i = 0; i < 6; i++)
  {
    if(vertex->neighbors[i] == target)
    {
      vertex->neighbors[i] = -1;
      return;
    }
  }
}

// replaces a default -1 value with the target value
// does nothing if the target already exists in the vertex or there are no more default values
void icoVertexAddNeighbor(IcoVertex* vertex, int neighbor)
{
  if(icoVertexFindNeighbor(vertex, neighbor) != -1)
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
void icoVertexReplaceNeighbor(IcoVertex* vertex, int cur, int next)
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

// populate default icosphere face data
void icoFaceDefault(IcoFace* face)
{
  memset(face->vertices, 0, sizeof(face->vertices));
  memset(face->indices, -1, sizeof(face->indices));
  memset(face->next, 0, sizeof(face->next));
}

// populate default icosphere vertex data
void icoVertexDefault(IcoVertex* vertex)
{
  vertex->idx = -1;
  memset(vertex->neighbors, -1, sizeof(vertex->neighbors));
  memset(vertex->normal, 0, sizeof(vertex->normal));
}

// finds which vertex index is neighbors to both provided indices
int icoVertexFindIntersection(IcoVertex* icoVertices, unsigned int vertexOne, unsigned int vertexTwo)
{
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
        return icoVertices[vertexOne].neighbors[i];
      }
    }
  }

  return -1;
}

// creates edges between all the vertices in an icosphere face
void icoFaceConnect(IcoFace* face, IcoVertex* vertices)
{
  for(int vertex = 0; vertex < 3; vertex++)
  {
    for(int neighbor = 0; neighbor < 3; neighbor++)
    {
      if(vertex == neighbor)
      {
        continue;
      }

      icoFacePrint(face);
      icoVertexAddNeighbor(vertices + face->indices[vertex], face->indices[neighbor]);
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

void icoFaceSubdivide(IcoFace* face, float defaultSize, IcoVertex* icoVertices, unsigned int* vertexIdx)
{
  // continual traversing tree if current face already subdivided
  if(face->next[0])
  {
    for(int i = 0; i < 4; i++)
    {
      icoFaceSubdivide(face->next[i], defaultSize, icoVertices, vertexIdx);
    }

    return;
  }

  // populate midpoint vertex metadata
  float midpointVertices[3][3]; // stores the coordinates of the midpoints
  unsigned int midpointIndices[3]; // stores the indices of the midpoints
  for(int midpoint = 0; midpoint < 3; midpoint++)
  {
    // the two face indices which form the edge the current midpoint is on
    unsigned int parentIdxOne = midpointPattern[midpoint][0];
    unsigned int parentIdxTwo = midpointPattern[midpoint][1];
    unsigned int parentVertexOne = face->indices[parentIdxOne];
    unsigned int parentVertexTwo = face->indices[parentIdxTwo];

    glm_vec3_add(face->vertices[parentIdxOne], face->vertices[parentIdxTwo], midpointVertices[midpoint]);
    glm_vec3_scale_as(midpointVertices[midpoint], defaultSize, midpointVertices[midpoint]);

    // if the parent vertices are still adjacent, the midpoint has not yet been created
    if(icoVertexFindNeighbor(icoVertices + parentVertexOne, parentVertexTwo) != -1)
    {
      // replace previous connections with new midpoint vertex
      icoVertexReplaceNeighbor(icoVertices + parentVertexOne, parentVertexTwo, *vertexIdx);
      icoVertexReplaceNeighbor(icoVertices + parentVertexTwo, parentVertexOne, *vertexIdx);

      // create new midpoint vertex data
      icoVertexDefault(icoVertices + *vertexIdx);
      icoVertices[*vertexIdx].idx = *vertexIdx;
      icoVertexAddNeighbor(icoVertices + *vertexIdx, parentVertexOne);
      icoVertexAddNeighbor(icoVertices + *vertexIdx, parentVertexTwo);
      midpointIndices[midpoint] = *vertexIdx;
      (*vertexIdx)++;
    }
    else // the midpoint has already been created and is adjacent to both parent vertices
    {
      // find which midpoint lies in the intersection of both face verices
      midpointIndices[midpoint] = icoVertexFindIntersection(icoVertices, parentVertexOne, parentVertexTwo);
    }
  }

  // populate next four sub faces
  for(int subFace = 0; subFace < 4; subFace++)
  {
    face->next[subFace] = malloc(sizeof(IcoFace));
    icoFaceDefault(face->next[subFace]);
    IcoFace* nextFace = face->next[subFace];

    for(int vertex = 0; vertex < 3; vertex++)
    {
      unsigned int loc = divisionPattern[subFace][vertex][0];
      unsigned int idx = divisionPattern[subFace][vertex][1];

      if(loc == 0) // parent's vertex
      {
        glm_vec3_copy(face->vertices[idx], nextFace->vertices[vertex]);
        nextFace->indices[vertex] = face->indices[idx];
      }
      else // new midpoint vertex
      {
        glm_vec3_copy(midpointVertices[idx], nextFace->vertices[vertex]);
        nextFace->indices[vertex] = midpointIndices[idx];
      }
    }

    icoFaceConnect(nextFace, icoVertices);
  }
}

void icoMeshPopulate(float* vertices, IcoFace* face, unsigned int* faceIdx)
{
  if(face->next[0])
  {
    for(int i = 0; i < 4; i++)
    {
      icoMeshPopulate(vertices, face->next[i], faceIdx);
    }

    return;
  }

  for(int i = 0; i < 3; i++)
  {
    glm_vec3_copy(face->vertices[i], vertices + *faceIdx);
    (*faceIdx) += 3;
  }

  printf("%d\n", *faceIdx);
  icoFacePrint(face);
}

// the number of vertices in the finished icosphere
unsigned int sphereIcoMeshVertexCount()
{
  unsigned int result = 12;

  // each subdivision produces no more than 3 times as many vertices
  // 6 new vertices for each vertex, but each new vertex is counted twice
  for(int i = 0; i < RECUR; i++)
  {
    result *= 3;
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

  // each triangle has 3 vertices and 3 floats per vertex
  result *= 3 * 3;

  return result;
}

void sphereIcoMesh(float* vertices)
{
  // generate icosphere circumscribed in sphere of default radius
  float goldenRatio = 1.61803398875f;
  float radiusToSideRatio = sqrt(goldenRatio * goldenRatio + 1.0f) / 2.0f;
  radiusToSideRatio = 1.0f / radiusToSideRatio;
  const float defaultSize = 0.5f;
  float sideLength = radiusToSideRatio * defaultSize;

  // upper pentagonal base
  float top[3] = {0.0f, defaultSize, 0.0f};
  float topBase[5][3];
  topBase[0][0] = 0.0f;
  topBase[0][1] = sideLength * 0.5f;
  topBase[0][2] = sqrt(0.5f * 0.5f - topBase[0][1] * topBase[0][1]);
  mat4 rot = GLM_MAT4_IDENTITY;
  glm_rotate_y(rot, glm_rad(72.0f), rot);
  for(int i = 1; i < 5; i++)
  {
    glm_mat4_mulv3(rot, topBase[i - 1], 1.0f, topBase[i]);
  }
  
  // lower pentagonal base
  float bottom[3] = {0.0f, -defaultSize, 0.0f};
  float bottomBase[5][3];
  glm_mat4_identity(rot);
  glm_rotate_y(rot, glm_rad(36.0f), rot);
  for(int i = 0; i < 5; i++)
  {
    glm_mat4_mulv3(rot, topBase[i], 1.0f, bottomBase[i]);
    bottomBase[i][1] = -topBase[i][1];
  }

  // indices corresponding to icosahedron
  unsigned int topIdx = 0;
  float topIndices[5];
  float bottomIndices[5];
  for(int i = 0; i < 5; i++)
  {
    topIndices[i] = i + 1;
    bottomIndices[i] = i + 6;
  }
  unsigned int bottomIdx = 11;

  // populate default array of icosphere faces
  IcoFace icoFaces[20];
  for(int i = 0; i < 20; i++)
  {
    icoFaceDefault(icoFaces + i);
  }

  // populate default array of icosphere vertices
  IcoVertex icoVertices[sphereIcoMeshVertexCount()];
  for(int i = 0; i < sphereIcoMeshVertexCount(); i++)
  {
    icoVertexDefault(icoVertices + i);
  }

  // populate initial icosahedron data
  for(int face = 0; face < 5; face++)
  {
    // upper pentagonal pyramid
    const float* curTopVertices[3] = {top, topBase[face], topBase[(face + 1) % 5]};
    const unsigned int curTopIndices[3] = {topIdx, topIndices[face], topIndices[(face + 1) % 5]};

    // lower pentagonal pyramid
    const float* curBottomVertices[3] = {bottom, bottomBase[(face + 1) % 5], bottomBase[face]};
    const unsigned int curBottomIndices[3] = {bottomIdx, bottomIndices[(face + 1) % 5], bottomIndices[face]};

    // up (triangle is right side up) middle faces
    const float* curUpVertices[3] = {topBase[face], bottomBase[(face + 4) % 5], bottomBase[face]};
    const unsigned int curUpIndices[3] = {topIndices[face], bottomIndices[(face + 4) % 5], bottomIndices[face]};

    // down (triangle is upside down) middle faces
    const float* curDownVertices[3] = {bottomBase[face], topBase[(face + 1) % 5], topBase[face]};
    const unsigned int curDownIndices[3] = {bottomIndices[face], topIndices[(face + 1) % 5], topIndices[face]};

    for(int vertex = 0; vertex < 3; vertex++)
    {
      // copy coordinate data
      glm_vec3_copy((float*) curTopVertices[vertex], icoFaces[face].vertices[vertex]);
      glm_vec3_copy((float*) curBottomVertices[vertex], icoFaces[face + 5].vertices[vertex]);
      glm_vec3_copy((float*) curUpVertices[vertex], icoFaces[face + 10].vertices[vertex]);
      glm_vec3_copy((float*) curDownVertices[vertex], icoFaces[face + 15].vertices[vertex]);

      // update current face to include index
      icoFaces[face].indices[vertex] = curTopIndices[vertex];
      icoFaces[face + 5].indices[vertex] = curBottomIndices[vertex];
      icoFaces[face + 10].indices[vertex] = curUpIndices[vertex];
      icoFaces[face + 15].indices[vertex] = curDownIndices[vertex];

      // update vertex index data
      icoVertices[curTopIndices[vertex]].idx = curTopIndices[vertex];
      icoVertices[curBottomIndices[vertex]].idx = curBottomIndices[vertex];
      icoVertices[curUpIndices[vertex]].idx = curUpIndices[vertex];
      icoVertices[curDownIndices[vertex]].idx = curDownIndices[vertex];

      // update neighbor adjacency data
      for(int neighbor = 0; neighbor < 3; neighbor++)
      {
        if(vertex == neighbor)
        {
          continue;
        }

        // create edges between vertices
        icoVertexAddNeighbor(icoVertices + curTopIndices[vertex], curTopIndices[neighbor]);
        icoVertexAddNeighbor(icoVertices + curBottomIndices[vertex], curBottomIndices[neighbor]);
        icoVertexAddNeighbor(icoVertices + curUpIndices[vertex], curUpIndices[neighbor]);
        icoVertexAddNeighbor(icoVertices + curDownIndices[vertex], curDownIndices[neighbor]);
      }
    }
  }

  // begin subdivisions
  unsigned int vertexIdx = 12; // the index the next faces will take during subdivision
  for(int i = 0; i < RECUR; i++)
  {
    for(int j = 0; j < 20; j++)
    {
      icoFaceSubdivide(icoFaces + j, defaultSize, icoVertices, &vertexIdx);
    }
  }

  printf("FACES\n");
  // populate resulting data into vertex buffer
  unsigned int faceIdx = 0; // the index the current face will have in the vertex buffer
  for(int i = 0; i < 20; i++)
  {
    icoMeshPopulate(vertices, icoFaces + i, &faceIdx);
  }

  printf("VERTICES\n");
  for(int i = 0; i < vertexIdx; i++)
  {
    printf("%d\n", icoVertices[i].idx);
    icoVertexPrint(icoVertices + i);
    printf("\n");
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


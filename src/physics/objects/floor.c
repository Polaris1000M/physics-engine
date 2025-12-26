#include "floor.h"

#include <cglm/cglm.h>

void floorMesh(float* mesh)
{
    const float defaultSize = 1.0f;

    // create floor
    const float signs[4][2] = {
        {-1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}};
    const int pattern[2][3] = {{3, 2, 0}, {3, 0, 1}};

    const unsigned int floatsPerVertex = 6;
    const unsigned int floatsPerTriangle = 3 * floatsPerVertex;
    vec3 normal = {0.0f, 1.0f, 0.0f};
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int idx = i * floatsPerTriangle + j * floatsPerVertex;
            vec3 coord = {signs[pattern[i][j]][0] * defaultSize, 0.0f,
                          signs[pattern[i][j]][1] * defaultSize};
            glm_vec3_copy(coord, mesh + idx);
            glm_vec3_copy(normal, mesh + idx + 3);
        }
    }
}

unsigned int floorMeshSize()
{
    // 6 floats per vertex
    // 18 floats per triangle
    // 2 triangles
    return 36;
}


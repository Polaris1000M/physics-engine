#include "cube.h"
#include <cglm/cglm.h>

void cubeMesh(float* vertices)
{
    const float defaultSize = 1.0f;

    // half of a side length of the square
    // all points in the cube have coordinates either positive or negative half
    // multiplied by square root of 3
    const float half = defaultSize / 1.73205081f;

    const int floatsPerVertex = 6;
    const int floatsPerTriangle = 3 * floatsPerVertex;

    // describes the sign of the other 2 axes for any fixed axis
    const float signs[4][2] =
    {
        {-1.0f, 1.0f},
        {1.0f, 1.0f},
        {-1.0f, -1.0f},
        {1.0f, -1.0f}
    };

    // for each of the 6 faces, describes the correct pattern to produce 
    // counterclockwise triangles
    const int patterns[6][2][3] = 
    {
        {
            {1, 3, 2},
            {1, 2, 0}
        },
        {
            {3, 1, 0},
            {3, 0, 2}
        },
        {
            {1, 0, 2},
            {1, 2, 3}
        },
        {
            {3, 2, 0},
            {3, 0, 1}
        },
        {
            {0, 1, 3},
            {0, 3, 2}
        },
        {
            {1, 0, 2},
            {1, 2, 3}
        }
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
                if(fixed == 0)
                {
                    face[sign][1] = half * signs[sign][0];
                    face[sign][2] = half * signs[sign][1];
                }
                else if(fixed == 1)
                {
                    face[sign][0] = half * signs[sign][0];
                    face[sign][2] = half * signs[sign][1];
                }
                else if(fixed == 2)
                {
                    face[sign][0] = half * signs[sign][0];
                    face[sign][1] = half * signs[sign][1];
                }
            }

            vec3 normal = {0.0f, 0.0f, 0.0f};
            normal[fixed] = 2 * fixedSign - 1.0f;
            for(int i = 0; i < 2; i++)
            {
                for(int j = 0; j < 3; j++)
                {
                    int patternIdx = fixed * 2 + fixedSign;
                    int idx = fixed * 4 * floatsPerTriangle + fixedSign * 2 * floatsPerTriangle + i * floatsPerTriangle + j * floatsPerVertex;
                    glm_vec3_copy(face[patterns[patternIdx][i][j]], vertices + idx);
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

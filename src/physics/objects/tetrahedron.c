#include "tetrahedron.h"
#include <math.h>
#include <cglm/cglm.h>
#include <string.h>

void tetrahedronMesh(float* vertices)
{
    float coords[4][3];

    const float defaultSize = 1.0f;
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

    const unsigned int floatsPerVertex = 6;
    const unsigned int floatsPerTriangle = floatsPerVertex * 3;

    for(int face = 0; face < 3; face++)
    {
        const unsigned int indices[3] = {3, face, (face + 1) % 3};
        vec3 e1, e2, normal;
        glm_vec3_sub(coords[face], coords[3], e1);
        glm_vec3_sub(coords[(face + 1) % 3], coords[3], e2);
        glm_vec3_crossn(e1, e2, normal);

        unsigned int idx = face * floatsPerTriangle;
        for(int i = 0; i < 3; i++)
        {
            glm_vec3_copy(coords[indices[i]], vertices + idx);
            glm_vec3_copy(normal, vertices + idx + 3);
            idx += floatsPerVertex;
        }
    }

    vec3 normal = {0.0f, -1.0f, 0.0f};
    for(int i = 0; i < 3; i++)
    {
        unsigned int idx = 3 * floatsPerTriangle + i * floatsPerVertex;
        glm_vec3_copy(coords[2 - i], vertices + idx);
        glm_vec3_copy(normal, vertices + idx + 3);
    }
}

unsigned int tetrahedronMeshSize()
{
    // 4 triangles
    // 18 floats per triangle
    return 72;
}

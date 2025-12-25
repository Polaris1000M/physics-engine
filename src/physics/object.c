#include "object.h"

#include <cglm/cglm.h>
#include <stdlib.h>
#include "cglm/vec3.h"
#include "utils/quat.h"
#include "cJSON.h"

const char* OBJECT_NAMES[] = {"floor", "sphere", "cube", "tetrahedron"};

void objectInit(Object* o, ObjectType type, float size, float mass,
                vec3 position, vec3 color)
{
    o->type = type;
    o->size = size;
    o->mass = mass;
    glm_vec3_copy(position, o->position);
    glm_vec3_copy(color, o->color);
}

void objectPrint(Object* o)
{
    printf("type: %s\n", OBJECT_NAMES[o->type]);
    printf("size: %f\n", o->size);
    printf("mass: %f\n", o->mass);
    printf("position:\n");
    glm_vec3_print(o->position, stdout);
    printf("color:\n");
    glm_vec3_print(o->color, stdout);
    printf("orientation:\n");
    glm_vec4_print(o->orientation, stdout);
}

void objectVertices(Object* o, float* vertices)
{
    mat4 res;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j)
            {
                res[i][j] = o->size;
            }
            else
            {
                res[i][j] = 0.0f;
            }
        }
    }
    res[3][3] = 1.0f;

    for (int i = 0; i < 3; i++)
    {
        res[i][3] = o->position[i];
    }

    mat4 rot;
    glm_quat_mat4(o->orientation, rot);
    glm_mat4_mul(rot, res, res);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vertices[i * 4 + j] = res[j][i];
        }
    }

    for (int i = 16; i < 19; i++)
    {
        vertices[i] = o->color[i - 16];
    }
}

unsigned int objectVerticesSize()
{
    // 4x4 float matrix and 3 floats for color
    return 19;
}

cJSON* objectToJSON(Object* o, float deltaTime)
{
    cJSON* configObject = cJSON_CreateObject();

    cJSON_AddStringToObject(configObject, "type", OBJECT_NAMES[o->type]);
    cJSON_AddNumberToObject(configObject, "size", o->size);
    cJSON_AddNumberToObject(configObject, "mass", o->mass);

    vec3 velocity;
    glm_vec3_sub(o->position, o->lastPosition, velocity);
    glm_vec3_scale(velocity, 1.0 / deltaTime, velocity);
    cJSON* configVelocity = cJSON_CreateFloatArray(velocity, 3);
    cJSON_AddItemReferenceToObject(configObject, "velocity", configVelocity);

    cJSON* configPosition = cJSON_CreateFloatArray(o->position, 3);
    cJSON_AddItemReferenceToObject(configObject, "position", configPosition);

    cJSON* configColor = cJSON_CreateFloatArray(o->color, 3);
    cJSON_AddItemReferenceToObject(configObject, "color", configColor);

    vec3 euler;
    quatToEuler(o->orientation, euler);
    cJSON* configOrientation = cJSON_CreateFloatArray(euler, 3);
    cJSON_AddItemReferenceToObject(configObject, "euler", configOrientation);
    
    return configObject;
}

#include "parse.h"

#include <string.h>

#include "../physics/object.h"
#include "../physics/physics.h"
#include "cJSON.h"
#include <cglm/cglm.h>
#include "cglm/vec3.h"
#include "utils/quat.h"

// parses a single vec3 based on cJSON array
unsigned int parseVec3(float* target, const cJSON* vec, const char* message)
{
    if (!cJSON_IsArray(vec) || cJSON_GetArraySize(vec) != 3)
    {
        printf("%s", message);
        return 1;
    }
    for (int i = 0; i < 3; i++)
    {
        cJSON* coord = cJSON_GetArrayItem(vec, i);
        if (!cJSON_IsNumber(coord))
        {
            printf("%s", message);
            return 1;
        }
        target[i] = coord->valuedouble;
    }
    return 0;
}

// parses a single JSON into an object
// expects type, size, mass, position, color, static (default false), velocity (default 0), spin (default 0), euler (default 0)
unsigned int parseConfigObject(cJSON* configObject, Object* object)
{
    // populate type since type already checked
    const cJSON* configType =
        cJSON_GetObjectItemCaseSensitive(configObject, "type");
    for (int i = 0; i < OBJECT_TYPES; i++)
    {
        if (!strcmp(configType->valuestring, OBJECT_NAMES[i]))
        {
            object->type = i;
            break;
        }
    }

    // parse size
    const cJSON* configSize =
        cJSON_GetObjectItemCaseSensitive(configObject, "size");
    if (!cJSON_IsNumber(configSize))
    {
        printf(
            "ERROR::CONFIG::INVALID_SIZE: expected float for size of object\n");
        return 1;
    }

    // populate mass according to type
    if (object->type != FLOOR)
    {
        const cJSON* configMass =
            cJSON_GetObjectItemCaseSensitive(configObject, "mass");
        if (!cJSON_IsNumber(configMass) || configMass->valuedouble <= 0.0f)
        {
            printf(
                "ERROR::CONFIG::INVALID_MASS: expected float for mass of "
                "object\n");
        }

        // populate mass
        object->mass = configMass->valuedouble;
    }

    // parse position
    const cJSON* configPosition =
        cJSON_GetObjectItemCaseSensitive(configObject, "position");
    const char* positionErrorMessage =
        "ERROR::CONFIG::INVALID_POSITION: expected float array for position of "
        "object with format [<x>, <y>, <z>]\n";
    vec3 position = GLM_VEC3_ZERO;
    if (parseVec3(position, configPosition, positionErrorMessage))
    {
        return 1;
    }

    // parse euler
    cJSON* configEuler =
        cJSON_GetObjectItemCaseSensitive(configObject, "euler");
    vec3 euler = GLM_VEC3_ZERO;
    if (configEuler)
    {
        const char* orientationErrorMessage =
            "ERROR::CONFIG::INVALID_EULER:: expected float array for "
            "euler angles of object in degrees with format [<pitch>, <yaw>, "
            "<roll>]\n";
        if (parseVec3(euler, configEuler, orientationErrorMessage))
        {
            printf("%s", orientationErrorMessage);
            return 1;
        }

        for (int i = 0; i < 3; i++)
        {
            euler[i] = glm_rad(euler[i]);
        }
    }

    // create quaternion from Euler angles
    eulerToQuat(euler, object->orientation);

    // parse color
    cJSON* configColor =
        cJSON_GetObjectItemCaseSensitive(configObject, "color");
    const char* colorErrorMessage =
        "ERROR::CONFIG::INVALID_COLOR: expected float array for color of "
        "object with format [<red_color>, <blue_color>, <green_color>] with "
        "non-negative values\n";
    if (!cJSON_IsArray(configColor) || cJSON_GetArraySize(configColor) != 3)
    {
        printf("%s", colorErrorMessage);
        return 1;
    }
    vec3 color;
    for (int i = 0; i < 3; i++)
    {
        cJSON* curColor = cJSON_GetArrayItem(configColor, i);

        if (!cJSON_IsNumber(curColor) || curColor->valuedouble < 0.0f)
        {
            printf("%s", colorErrorMessage);
            return 1;
        }

        color[i] = curColor->valuedouble;
    }

    unsigned int scale = 0;
    for (int i = 0; i < 3; i++)
    {
        if (color[i] > 1)
        {
            scale = 1;
            break;
        }
    }
    if (scale)
    {
        for (int i = 0; i < 3; i++)
        {
            color[i] /= 255.0f;
        }
    }

    // parse static
    cJSON* configStatic =
        cJSON_GetObjectItemCaseSensitive(configObject, "static");
    const char* staticErrorMessage =
        "ERROR::CONFIG::INVALID_STATIC: expected boolean\n";
    if (configStatic && !cJSON_IsBool(configStatic))
    {
        printf("%s", staticErrorMessage);
        return 1;
    }
    int staticPhysics;
    if (configStatic)
    {
        staticPhysics = configStatic->valueint;
    }
    else
    {
        staticPhysics = 0;
    }

    // parse velocity
    cJSON* configVelocity =
        cJSON_GetObjectItemCaseSensitive(configObject, "velocity");
    const char* velocityErrorMessage =
        "ERROR::CONFIG::INVALID_VELOCITY: expected vector of three floats\n";
    vec3 velocity = GLM_VEC3_ZERO;
    if (configVelocity &&
        parseVec3(velocity, configVelocity, velocityErrorMessage))
    {
        return 1;
    }

    // parse spin
    cJSON* configSpin =
        cJSON_GetObjectItemCaseSensitive(configObject, "spin");
    const char* spinErrorMessage =
        "ERROR::CONFIG::INVALID_SPIN: expected vector of three floats\n";
    vec3 spin = GLM_VEC3_ZERO;
    if (configSpin &&
        parseVec3(spin, configSpin, spinErrorMessage))
    {
        return 1;
    }

    // populate size
    object->size = configSize->valuedouble;

    // populate position
    glm_vec3_copy(position, object->position);

    // populate color
    glm_vec3_copy(color, object->color);

    // populate static
    object->staticPhysics = staticPhysics;

    // populate last position
    glm_vec3_scale(velocity, PHYSICS_DT, velocity);
    glm_vec3_sub(position, velocity, object->lastPosition);

    // populate spin
    glm_vec3_copy(spin, object->angularVelocity);
    glm_vec3_copy(object->angularAcceleration, GLM_VEC3_ZERO);

    return 0;
}

// parses cJSON array into array of objects
unsigned int parseConfigObjects(cJSON* configObjects,
                                unsigned int* objectCounts, Object** objects)
{
    // determines number of each type of object to properly allocate object
    // array then parses each object individually

    unsigned int numObjects = cJSON_GetArraySize(configObjects);
    const char* objectsErrorMessage =
        "ERROR::CONFIG::INVALID_OBJECTS: expected non-empty JSON array of "
        "objects\n";
    if (numObjects == 0)
    {
        printf("%s", objectsErrorMessage);
        return 1;
    }

    // create type error message
    unsigned int typeErrorMessageSize = 38;  // front of message
    typeErrorMessageSize += 19;              // end of message
                                 // 2 quotes for each name, a comma and a
                                 // space, an additional or
    typeErrorMessageSize += OBJECT_TYPES * 2 + (OBJECT_TYPES - 1) * 2 + 3;
    for (unsigned int type = 0; type < OBJECT_TYPES; type++)
    {
        typeErrorMessageSize += strlen(OBJECT_NAMES[type]);
    }
    // increment for null-terminated character at end
    typeErrorMessageSize++;

    char* typeErrorMessage = malloc(typeErrorMessageSize * sizeof(char));
    typeErrorMessage[0] = '\0';
    strcat(typeErrorMessage, "ERROR::CONFIG::INVALID_TYPE: expected ");
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        if (type == OBJECT_TYPES - 1)
        {
            strcat(typeErrorMessage, ", or \"");
        }
        else if (type != 0)
        {
            strcat(typeErrorMessage, ", \"");
        }
        strcat(typeErrorMessage, OBJECT_NAMES[type]);
        strcat(typeErrorMessage, "\"");
    }
    strcat(typeErrorMessage, " for type of object\n");

    // initialize each count to 0
    for (int i = 0; i < OBJECT_TYPES; i++)
    {
        objectCounts[i] = 0;
    }

    // get object counts
    for (int i = 0; i < numObjects; i++)
    {
        cJSON* configObject = cJSON_GetArrayItem(configObjects, i);

        // parse the type of the object (i.e., cube, sphere, tetrahedron, etc.)
        const cJSON* configType =
            cJSON_GetObjectItemCaseSensitive(configObject, "type");

        if (!cJSON_IsString(configType))
        {
            printf("%s", typeErrorMessage);
            return 1;
        }

        // update object type
        int match = 0;
        for (int type = 0; type < OBJECT_TYPES; type++)
        {
            if (!strcmp(configType->valuestring, OBJECT_NAMES[type]))
            {
                objectCounts[type]++;
                match = 1;
                break;
            }
        }
        if (!match)
        {
            printf("%s", typeErrorMessage);
            return 1;
        }
    }

    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        objects[type] = malloc(objectCounts[type] * sizeof(Object));
    }

    unsigned int indices[OBJECT_TYPES];
    memset(indices, 0, OBJECT_TYPES * sizeof(unsigned int));
    for (int i = 0; i < numObjects; i++)
    {
        cJSON* configObject = cJSON_GetArrayItem(configObjects, i);
        const cJSON* configType =
            cJSON_GetObjectItemCaseSensitive(configObject, "type");
        for (int type = 0; type < OBJECT_TYPES; type++)
        {
            if (!strcmp(configType->valuestring, OBJECT_NAMES[type]))
            {
                if (parseConfigObject(configObject,
                                      objects[type] + indices[type]))
                {
                    return 1;
                }
                indices[type]++;
                break;
            }
        }
    }

    return 0;
}

unsigned int parseConfig(Simulation* sim, const char* configPath)
{
    // parse config file
    char* configData = parseFile(configPath, "CONFIG");
    if (!configData)
    {
        return 1;
    }
    cJSON* config = cJSON_Parse(configData);

    const cJSON* gravity = cJSON_GetObjectItemCaseSensitive(config, "gravity");
    if (!cJSON_IsNumber(gravity))
    {
        printf("ERROR::CONFIG::INVALID_GRAVITY: expected float\n");
        return 1;
    }
    sim->gravity = gravity->valuedouble;

    const cJSON* light = cJSON_GetObjectItemCaseSensitive(config, "lightDir");
    const char* lightMessage =
        "ERROR::CONFIG::INVALID_LIGHT_DIR: expected float array with format "
        "[<x>, <y>, <z>] for light direction\n";
    if (parseVec3(sim->lightDir, light, lightMessage))
    {
        return 1;
    }
    glm_vec3_normalize(sim->lightDir);

    const cJSON* cameraPos =
        cJSON_GetObjectItemCaseSensitive(config, "cameraPos");
    const char* cameraPosMessage =
        "ERROR::CONFIG::INVALID_CAMERA_POS: expected float array with format "
        "[<x>, <y>, <z>] for camera position\n";
    if (parseVec3(sim->camera.cameraPos, cameraPos, cameraPosMessage))
    {
        return 1;
    }

    const cJSON* cameraDir =
        cJSON_GetObjectItemCaseSensitive(config, "cameraDir");
    const char* cameraDirMessage =
        "ERROR::CONFIG::INVALID_CAMERA_DIR: expected float array with format "
        "[<x>, <y>, <z>] for camera direction\n";
    if (parseVec3(sim->camera.cameraFront, cameraDir, cameraDirMessage))
    {
        return 1;
    }
    glm_vec3_normalize(sim->camera.cameraFront);

    sim->gravity = gravity->valuedouble;

    cJSON* configObjects = cJSON_GetObjectItemCaseSensitive(config, "objects");

    if (parseConfigObjects(configObjects, sim->objectCounts, sim->objects))
    {
        return 1;
    }

    return 0;
}

char* parseFile(const char* filePath, const char* errorMessage)
{
    FILE* fp;
    char* buffer;

    fp = fopen(filePath, "rb");
    if (!fp)
    {
        printf("ERROR::%s::FILE_NOT_SUCCESSFULLY_READ: %s\n", errorMessage,
               filePath);
        buffer = 0;
        return buffer;
    }

    // find length of file
    fseek(fp, 0L, SEEK_END);
    long shaderSize = ftell(fp);
    rewind(fp);

    buffer = calloc(1, shaderSize + 1);

    if (!buffer)
    {
        printf("ERROR::%s::FAILED_TO_ALLOCATE_BUFFER: %s\n", errorMessage,
               filePath);
        buffer = 0;
        return buffer;
    }

    if (fread(buffer, shaderSize, 1, fp) != 1)
    {
        printf("ERROR::%s::FILE_NOT_SUCCESSFULLY_READ: %s\n", errorMessage,
               filePath);
        buffer = 0;
        return buffer;
    }

    return buffer;
}

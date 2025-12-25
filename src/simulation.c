#include "simulation.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cJSON.h"
#include "physics/physics.h"
#include "physics/object.h"
#include "render/camera.h"
#include "render/render.h"
#include "utils/callbacks.h"
#include "utils/parse.h"

unsigned int simulationInit(Simulation* sim, const char* configPath)
{
    sim->configPath = configPath;
    sim->avgFPS = 0;
    sim->frames = 0;
    sim->lastTime = 0.0f;
    sim->timeRatio = 0.5f;
    sim->physicsDeltaTime = 1.0 / 60.0f;

    // initialize objects from config
    if (parseConfig(sim, configPath))
    {
        return 1;
    }

    renderInit(sim);
    callbacksInit(sim);

    return 0;
}

// processes keyboard and mouse input from use
void simulationProcessInput(Simulation* sim)
{
    if (glfwGetKey(sim->window, GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(sim->window, 1);
    }

    if (glfwGetKey(sim->window, GLFW_KEY_ENTER))
    {
        simulationSave(sim);
    }

    cameraProcessInput(&sim->camera, sim->window);
}

void simulationUpdate(Simulation* sim)
{
    simulationProcessInput(sim);

    renderUpdate(sim);
    sim->frames++;
    physicsUpdate(sim);
}

void simulationFree(Simulation* sim)
{
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        free(sim->objects[type]);
        free(sim->meshes[type]);
        free(sim->objectData[type]);
    }

    glDeleteFramebuffers(1, &sim->shadow.FBO);
    glDeleteBuffers(3, sim->meshVBOs);
    glDeleteVertexArrays(3, sim->VAOs);
    glDeleteProgram(sim->shader.ID);
}

void simulationStart(Simulation* sim)
{
    // simulationPrint(sim);
    while (!glfwWindowShouldClose(sim->window))
    {
        simulationUpdate(sim);
        render(sim);

        glfwSwapBuffers(sim->window);
        glfwPollEvents();
    }

    glfwTerminate();
    simulationFree(sim);
}

void simulationSave(Simulation* sim)
{
    cJSON* config = cJSON_CreateObject();

    // save simulation configuration
    cJSON_AddNumberToObject(config, "gravity", sim->gravity);

    cJSON* configLightDir = cJSON_CreateFloatArray(sim->lightDir, 3);
    cJSON_AddItemReferenceToObject(config, "lightDir", configLightDir);

    cJSON* configCameraDir = cJSON_CreateFloatArray(sim->camera.cameraFront, 3);
    cJSON_AddItemReferenceToObject(config, "cameraDir", configCameraDir);

    cJSON* configCameraPos = cJSON_CreateFloatArray(sim->camera.cameraPos, 3);
    cJSON_AddItemReferenceToObject(config, "cameraPos", configCameraPos);

    // save simulation objects
    cJSON* configObjects = cJSON_CreateArray();
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        for (int i = 0; i < sim->objectCounts[type]; i++)
        {
            cJSON* configObject = objectToJSON(sim->objects[type] + i, sim->physicsDeltaTime);
            cJSON_AddItemToArray(configObjects, configObject);
        }
    }

    cJSON_AddItemReferenceToObject(config, "objects", configObjects);

    char* configString = cJSON_Print(config);

    FILE* configFile = fopen("../configs/saved.json", "w");
    fprintf(configFile, "%s", configString);
    fclose(configFile);
    free(configString);
}

void simulationPrint(Simulation* sim)
{
    printf("OBJECTS\n");
    printf("Object Counts\n");
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        printf("%s: %d\n", OBJECT_NAMES[type], sim->objectCounts[type]);
    }
    printf("\n");

    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        printf("%s\n", OBJECT_NAMES[type]);

        for (int i = 0; i < sim->objectCounts[type]; i++)
        {
            objectPrint(sim->objects[type] + i);
            printf("\n");
        }
        printf("\n");
    }

    printf("MESHES\n");
    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        if (type == SPHERE)
        {
            continue;
        }
        printf("%s %d\n", OBJECT_NAMES[type], sim->meshSizes[type]);
        for (int i = 0; i < sim->meshSizes[type]; i += 3)
        {
            printf("(%f, %f, %f)\n", sim->meshes[type][i],
                   sim->meshes[type][i + 1], sim->meshes[type][i + 2]);
        }
        printf("\n");
    }

    // printf("VERTICES\n");
    // for(int type = 0; type < OBJECT_TYPES; type++)
    // {
    //   printf("%s: %d\n", OBJECT_NAMES[type], sim->vertexCounts[type]);
    // }
    // for(int type = 0; type < OBJECT_TYPES; type++)
    // {
    //   for(int i = 0; i < sim->objectCounts[type]; i++)
    //   {
    //     for(int j = 0; j < objectVerticesSize(); j++)
    //     {
    //       printf("%f ", sim->vertices[type][i * objectVerticesSize() + j]);
    //     }
    //     printf("\n");
    //   }
    //   printf("\n");
    // }

    // printf("Single Vertex Counts\n");
    // for(int type = 0; type < OBJECT_TYPES; type++)
    // {
    //   printf("%s: %d\n", OBJECT_NAMES[type], sim->singleVertexCounts[type]);
    // }
    // printf("\n");

    // printf("Vertex Counts\n");
    // for(int type = 0; type < OBJECT_TYPES; type++)
    // {
    //   printf("%s: %d\n", OBJECT_NAMES[type], sim->vertexCounts[type]);
    // }
    // printf("\n");

    // for(int type = 0; type < OBJECT_TYPES; type++)
    // {
    //   printf("%s\n", OBJECT_NAMES[type]);
    //   // print all vertices in a single object
    //   for(int i = 0; i < sim->vertexCounts[type]; i += 3)
    //   {
    //     printf("(%f, %f, %f)\n", sim->vertices[type][i],
    //     sim->vertices[type][i + 1], sim->vertices[type][i + 2]);
    //     // // iterate over all groups of 3 vertices in the current object
    //     // for(int j = 0; j < i * sim->singleVertexCounts[type]; j += 3)
    //     // {
    //     //   printf("(%f, %f, %f)\n", sim->vertices[type][j],
    //     sim->vertices[type][j + 1], sim->vertices[type][j + 2]);
    //     // }
    //   }
    // }

    // cameraPrint(&sim->camera);
    // shadowPrint(&sim->shadow);
}


#include "simulation.h"

#include <unistd.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON.h"
#include "physics/object.h"
#include "physics/objects/cube.h"
#include "physics/objects/floor.h"
#include "physics/objects/sphere.h"
#include "physics/objects/tetrahedron.h"
#include "render/camera.h"
#include "render/text.h"
#include "utils/parse.h"
#include "utils/save.h"
#include "utils/callbacks.h"

// initializes OpenGL and GLFW boilerplate
unsigned int openglInit(Simulation* sim)
{
    const char* openglInitErrorMessage =
        "ERROR::OPENGL::INITIALIZATION_ERROR: failed to initialize OpenGL and "
        "GLFW\n";

    if (!glfwInit())
    {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    sim->WINDOW_WIDTH = mode->width;
    sim->WINDOW_HEIGHT = mode->height;
    sim->window = glfwCreateWindow(sim->WINDOW_WIDTH, sim->WINDOW_HEIGHT,
                                   "PhysicsEngine", NULL, NULL);
    if (!sim->window)
    {
        glfwTerminate();
        printf("%s", openglInitErrorMessage);
        return 1;
    }

    glfwMakeContextCurrent(sim->window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("%s", openglInitErrorMessage);
        return 1;
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    return 0;
}

// generate and bind all object data (model matrices, color, and meshes) to
// OpenGL
void buffersInit(Simulation* sim)
{
    sim->meshSizes[FLOOR] = floorMeshSize();
    sim->meshSizes[SPHERE] = sphereIcoMeshSize();
    sim->meshSizes[CUBE] = cubeMeshSize();
    sim->meshSizes[TETRAHEDRON] = tetrahedronMeshSize();

    // stores each of the methods for generating meshes for easier iteration
    void (*generateMesh[OBJECT_TYPES])(float*);
    generateMesh[FLOOR] = floorMesh;
    generateMesh[SPHERE] = sphereIcoMesh;
    generateMesh[CUBE] = cubeMesh;
    generateMesh[TETRAHEDRON] = tetrahedronMesh;

    glGenVertexArrays(OBJECT_TYPES, sim->VAOs);
    glGenBuffers(OBJECT_TYPES, sim->meshVBOs);
    glGenBuffers(OBJECT_TYPES, sim->objectVBOs);

    for (int type = 0; type < OBJECT_TYPES; type++)
    {
        // generate default mesh data
        sim->meshes[type] = malloc(sim->meshSizes[type] * sizeof(float));
        generateMesh[type](sim->meshes[type]);

        // allocate object data
        sim->objectSizes[type] = sim->objectCounts[type] * objectVerticesSize();
        sim->objectData[type] = malloc(sim->objectSizes[type] * sizeof(float));

        glBindVertexArray(sim->VAOs[type]);

        // mesh vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, sim->meshVBOs[type]);
        glBufferData(GL_ARRAY_BUFFER, sim->meshSizes[type] * sizeof(float),
                     sim->meshes[type], GL_STATIC_DRAW);

        // position data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);

        // vertex normals
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(6);

        // object vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, sim->objectVBOs[type]);
        glBufferData(GL_ARRAY_BUFFER, sim->objectSizes[type] * sizeof(float),
                     sim->objectData[type], GL_STATIC_DRAW);

        // model matrix
        for (unsigned int i = 0; i < 4; i++)
        {
            glEnableVertexAttribArray(i + 1);
            glVertexAttribPointer(i + 1, 4, GL_FLOAT, GL_FALSE,
                                  objectVerticesSize() * sizeof(float),
                                  (void*)(i * 4 * sizeof(float)));
            glVertexAttribDivisor(i + 1, 1);  // configured for instancing
        }
        // object color
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE,
                              objectVerticesSize() * sizeof(float),
                              (void*)(16 * sizeof(float)));
        glVertexAttribDivisor(5, 1);

        glBindVertexArray(0);
    }
}

unsigned int simulationInit(Simulation* sim, const char* configPath)
{
    // OpenGL boilerplate
    if (openglInit(sim))
    {
        return 1;
    }

    // initialize objects from config
    if (parseConfig(sim, configPath))
    {
        return 1;
    }

    sim->lastTime = 0.0f;
    sim->timeRatio = 0.5f;

    shaderInit(&sim->shader, "../src/render/shaders/default-vs.glsl",
               "../src/render/shaders/default-fs.glsl");
    cameraInit(&sim->camera, sim->window);
    if (shadowInit(&sim->shadow, &sim->camera, sim->lightDir))
    {
        return 1;
    }
    if (textInit(&sim->text, "../assets/roboto/static/Roboto-Light.ttf"))
    {
        return 1;
    }

    // initalize object data and bind
    buffersInit(sim);

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

    if (glfwGetKey(sim->window, GLFW_KEY_P))
    {
        simulationPrint(sim);
    }

    cameraProcessInput(&sim->camera, sim->window);
}

void simulationUpdate(Simulation* sim)
{
    simulationProcessInput(sim);

    cameraUpdate(&sim->camera);
    shadowUpdate(&sim->shadow, &sim->camera);
}

// iterate through objects and render with instancing
void objectsRender(Simulation* sim)
{
    for (unsigned int type = 0; type < OBJECT_TYPES; type++)
    {
        // floor should not be culled
        if (type == FLOOR)
        {
            glDisable(GL_CULL_FACE);
        }

        glBindVertexArray(sim->VAOs[type]);
        for (unsigned int i = 0, idx = 0; i < sim->objectCounts[type];
             i++, idx += objectVerticesSize())
        {
            // update object model matrices and color
            objectVertices(&sim->objects[type][i], sim->objectData[type] + idx);
        }

        // reattach new object data
        glBindBuffer(GL_ARRAY_BUFFER, sim->objectVBOs[type]);
        glBufferData(GL_ARRAY_BUFFER, sim->objectSizes[type] * sizeof(float),
                     sim->objectData[type], GL_STATIC_DRAW);

        // draw objects with instancing
        glDrawArraysInstanced(GL_TRIANGLES, 0, sim->meshSizes[type] / 6,
                              sim->objectCounts[type]);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (type == FLOOR)
        {
            glEnable(GL_CULL_FACE);
        }
    }
}

void simulationRender(Simulation* sim)
{
    // shadow pass
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, sim->shadow.SHADOW_WIDTH, sim->shadow.SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, sim->shadow.FBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    shaderUse(&sim->shadow.shader);
    shaderSetMatrix(&sim->shadow.shader, "vp", sim->shadow.vp);
    objectsRender(sim);
    if (glfwGetKey(sim->window, GLFW_KEY_P))
    {
        saveFramebuffer(sim->shadow.FBO, sim->shadow.SHADOW_WIDTH,
                        sim->shadow.SHADOW_HEIGHT);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // normal lighting pass
    shaderUse(&sim->shader);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    shaderSetMatrix(&sim->shader, "vp", sim->camera.vp);
    shaderSetMatrix(&sim->shader, "shadowVP", sim->shadow.vp);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sim->shadow.depthMap);

    shaderSetInt(&sim->shader, "depthMap", 1);
    shaderSetVector(&sim->shader, "lightDir", sim->lightDir);
    shaderSetVector(&sim->shader, "viewPos", sim->camera.cameraPos);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    objectsRender(sim);

    // render metrics
    char buffers[OBJECT_TYPES + 2][20];
    char* text[OBJECT_TYPES + 2];
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - sim->lastTime;
    sim->lastTime = currentTime;
    snprintf(buffers[0], 20, "%f ms", deltaTime);
    int totalObjects = 0;
    for(int i = 0; i < OBJECT_TYPES; i++)
    {
        char name[20];
        strncpy(name, OBJECT_NAMES[i], 19);
        name[19] = '\0';
        name[0] = (char) (name[0] - 'a' + 'A');
        if (sim->objectCounts[i] != 1)
        {
            snprintf(buffers[i + 1], 20, "%d %ss", sim->objectCounts[i], name);
        }
        else
        {
            snprintf(buffers[i + 1], 20, "%d %s", sim->objectCounts[i], name);
        }
        totalObjects += sim->objectCounts[i];
    }
    
    if (totalObjects != 1)
    {
        snprintf(buffers[OBJECT_TYPES + 1], 20, "%d %ss", totalObjects, "Total Object");
    }
    else
    {
        snprintf(buffers[OBJECT_TYPES + 1], 20, "%d %s", totalObjects, "Total Object");
    }

    for(int i = 0; i <= OBJECT_TYPES + 1; i++)
    {
        text[i] = buffers[i];
    }
    textRender(&sim->text, OBJECT_TYPES + 2, text, sim->camera.WINDOW_WIDTH, sim->camera.WINDOW_HEIGHT, 25.0f, 25.0f, 1.0f, (vec3) { 0.0f, 0.0f, 0.0f });
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
    while (!glfwWindowShouldClose(sim->window))
    {
        simulationUpdate(sim);
        simulationRender(sim);

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
            cJSON* configObject = objectToJSON(sim->objects[type] + i);
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

#include "render.h"

#include <string.h>

#include "../simulation.h"
#include "physics/objects/cube.h"
#include "physics/objects/floor.h"
#include "physics/objects/sphere.h"
#include "physics/objects/tetrahedron.h"
#include "render/text.h"
#include "utils/save.h"

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

    // multisampling for anti-aliasing
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // insert dummy values into width and height since window will be instantly
    // maximized
    sim->window = glfwCreateWindow(1, 1, "PhysicsEngine", NULL, NULL);
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

unsigned int renderInit(Simulation* sim)
{
    // OpenGL boilerplate
    if (sim->initialized != 1)
    {
        if (openglInit(sim))
        {
            return 1;
        }
        shaderInit(&sim->shader, "../src/render/shaders/default-vs.glsl",
                   "../src/render/shaders/default-fs.glsl");
        if (shadowInit(&sim->shadow, &sim->camera, sim->lightDir))
        {
            return 1;
        }

        if (textInit(&sim->text, "../assets/roboto/static/Roboto-Light.ttf"))
        {
            return 1;
        }
    }

    cameraInit(&sim->camera, sim->window);

    // initalize object data and bind
    buffersInit(sim);

    return 0;
}

// updates all rendering related simulation components
void renderUpdate(Simulation* sim)
{
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

void render(Simulation* sim)
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
    unsigned int lines = OBJECT_TYPES + 6;
    char buffers[lines][20];
    char* text[lines];

    // object counts
    int totalObjects = 0;
    for (int i = 0; i < OBJECT_TYPES; i++)
    {
        char name[20];
        strncpy(name, OBJECT_NAMES[i], 19);
        name[19] = '\0';
        name[0] = (char)(name[0] - 'a' + 'A');
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
        snprintf(buffers[0], 20, "%d %ss", totalObjects, "Total Object");
    }
    else
    {
        snprintf(buffers[0], 20, "%d %s", totalObjects, "Total Object");
    }

    // FPS
    float currentTime = glfwGetTime();
    float fps = 1.0f / (currentTime - sim->lastTime);
    sim->lastTime = currentTime;
    snprintf(buffers[OBJECT_TYPES + 1], 20, "%f fps", fps);

    sim->avgFPS = ((sim->avgFPS * sim->frames) + fps) / (sim->frames + 1);
    snprintf(buffers[OBJECT_TYPES + 2], 20, "%f avg fps", sim->avgFPS);

    // fov
    snprintf(buffers[OBJECT_TYPES + 3], 20, "%.2f fov", sim->camera.fov);

    // time
    snprintf(buffers[OBJECT_TYPES + 4], 20, "%.2fs elapsed", currentTime);

    // frames
    snprintf(buffers[OBJECT_TYPES + 5], 20, "%llu frames", sim->frames);

    for (int i = 0; i < lines; i++)
    {
        text[i] = buffers[i];
    }
    textRender(&sim->text, lines, text, sim->camera.WINDOW_WIDTH,
               sim->camera.WINDOW_HEIGHT, 25.0f, 25.0f, 1.0f,
               (vec3){0.0f, 0.0f, 0.0f});
}


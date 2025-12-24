#include "../simulation.h"
#include "render.h"
#include "physics/objects/cube.h"
#include "physics/objects/floor.h"
#include "physics/objects/sphere.h"
#include "physics/objects/tetrahedron.h"

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

    // insert dummy values into width and height since window will be instantly maximized
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
    if (openglInit(sim))
    {
        return 1;
    }

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

    return 0;
}

// updates all rendering related simulation components
void renderUpdate(Simulation* sim)
{
    cameraUpdate(&sim->camera);
    shadowUpdate(&sim->shadow, &sim->camera);
}


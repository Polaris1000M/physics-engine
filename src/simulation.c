#include "simulation.h"
#include "cJSON.h"
#include "physics/sphere.h"
#include "physics/cube.h"
#include "physics/pyramid.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// reads config.json file into C-string
char* readConfig(const char* configPath)
{
  FILE* fp;
  char* buffer;
  
  fp = fopen(configPath, "rb");
  if(!fp)
  {
    printf("ERROR::CONFIG::FILE_NOT_SUCCESSFULLY_READ: %s\n", configPath);
    buffer = 0; 
    return buffer;
  }

  // find length of file
  fseek(fp, 0L, SEEK_END);
  long shaderSize = ftell(fp);
  rewind(fp);

  buffer = calloc(1, shaderSize + 1);

  if(!buffer)
  {
    printf("ERROR::CONFIG::FAILED_TO_ALLOCATE_BUFFER: %s\n", configPath);
    buffer = 0;
    return buffer;
  }

  if(fread(buffer, shaderSize, 1, fp) != 1)
  {
    printf("ERROR::CONFIG::FILE_NOT_SUCCESSFULLY_READ: %s\n", configPath);
    buffer = 0; 
    return buffer;
  }

  return buffer;
}

// parses an object and stores in simulation, returns 1 if failed and 0 otherwise
// i is the number in the original config file and idx is the index into the respective object array
int parseObject(Simulation* sim, cJSON* object, unsigned int idx, unsigned int sphereIdx, unsigned int cubeIdx, unsigned int pyramidIdx)
{
  const cJSON* type = cJSON_GetObjectItemCaseSensitive(object, "type");
  const cJSON* size = cJSON_GetObjectItemCaseSensitive(object, "size");
  const cJSON* mass = cJSON_GetObjectItemCaseSensitive(object, "mass");
  const cJSON* distribution = cJSON_GetObjectItemCaseSensitive(object, "distribution");
  const cJSON* position = cJSON_GetObjectItemCaseSensitive(object, "position");
  const cJSON* count = cJSON_GetObjectItemCaseSensitive(object, "count");

  if(!cJSON_IsNumber(size)) {
    printf("ERROR::CONFIG::INVALID_SIZE: expected float for size of object %d\n", idx + 1);
    return 1;
  }

  if(!cJSON_IsNumber(mass)) {
    printf("ERROR::CONFIG::INVALID_SIZE: expected float for mass of object %d\n", idx + 1);
    return 1;
  }

  if(!cJSON_IsNumber(count)) {
    printf("ERROR::CONFIG::INVALID_COUNT: expected integer for count of object %d\n", idx + 1);
    return 1;
  }

  if(!cJSON_IsString(distribution))
  {
    printf("ERROR::CONFIG::INVALID_DISTRIBUTION: expected string for distribution of object %d\n", idx + 1);
    return 1;
  }
  else if(strcmp(distribution->valuestring, "uniform") && strcmp(distribution->valuestring, "random"))
  {
    printf("ERROR::CONFIG::INVALID_DISTRIBUTION: expected \"random\" or \"uniform\" for distribution of object %d\n", idx + 1);
    return 1;
  }

  if(!cJSON_IsString(type))
  {
    printf("ERROR::CONFIG::INVALID_TYPE: expected string for type of object %d\n", idx + 1);
    return 1;
  }

  float positionRanges[3][2];
  if(!cJSON_IsArray(position))
  {
    printf("ERROR::CONFIG::INVALID_POSITION: expected array for position range of object %d\n", idx + 1);
    return 1;
  }

  unsigned int numCoords = cJSON_GetArraySize(position);
  if(numCoords != 3)
  {
    printf("ERROR::CONFIG::INVALID_POSITION: expected array of 3 coordinates for position range of object %d\n", idx + 1);
    return 1;
  }

  for(int i = 0; i < 3; i++)
  {
    cJSON* range = cJSON_GetArrayItem(position, i);

    unsigned int numRange = cJSON_GetArraySize(range);

    if(numRange != 2)
    {
      printf("ERROR::CONFIG::INVALID_POSITION: expected range of 2 values for position range of object %d\n", idx + 1);
      return 1;
    }

    cJSON* lower = cJSON_GetArrayItem(range, 0);
    cJSON* upper = cJSON_GetArrayItem(range, 1);

    if(!cJSON_IsNumber(lower) || !cJSON_IsNumber(upper))
    {
      printf("ERROR::CONFIG::INVALID_POSITION: expected numerical values for position range of object %d\n", idx + 1);
      return 1;
    }

    positionRanges[i][0] = lower->valuedouble;
    positionRanges[i][1] = upper->valuedouble;
  }

  // printf("%f %f %f %f %f %f\n", positionRanges[0][0], positionRanges[0][1], positionRanges[1][0], positionRanges[1][1], positionRanges[2][0], positionRanges[2][1]);

  for(unsigned int i = 0; i < count->valueint; i++)
  {
    vec3 currentPosition;
    if(strcmp(type->valuestring, "sphere") == 0)
    {
      if(strcmp(distribution->valuestring, "random") == 0)
      {
        for(unsigned int j = 0; j < 3; j++)
        {
          currentPosition[j] = positionRanges[j][0] + ((float) rand() / (float) RAND_MAX) * (positionRanges[j][1] - positionRanges[j][0]);
        }
      }
      else
      {
        for(unsigned int j = 0; j < 3; j++)
        {
          if(count->valueint == 1)
          {
            currentPosition[j] = (positionRanges[j][0] + positionRanges[j][1]) / 2.0f;
          }
          else
          {
            currentPosition[j] = (positionRanges[j][1] - positionRanges[j][0]) / (count->valuedouble - 1.0f) * (float) i + positionRanges[j][0];
          }
        }
      }

      sphereInit(sim->spheres + sphereIdx, size->valuedouble, mass->valuedouble, currentPosition, 10, 10);
      sphereIdx++;
    }
    else if(strcmp(type->valuestring, "cube") == 0)
    {
    }
    else if(strcmp(type->valuestring, "pyramid") == 0)
    {
    }
    else
    {
      printf("ERROR::CONFIG::INVALID_TYPE: expected \"sphere\", \"cube\", or \"pyramid\" for type of object %d instead of %s\n", idx + 1, type->valuestring);
      return 1;
    }
  }

  return 0;
}

void simulationUpdateCamera(Simulation* sim)
{
  // updates view to match where camera is currently pointing
  mat4 view;
  cameraLookAt(&sim->camera, view);
  shaderSetMatrix(&sim->shader, "view", view);

  // creates perspective
  mat4 projection = GLM_MAT4_IDENTITY;
  glm_perspective(glm_rad(sim->camera.fov), (float) sim->WINDOW_WIDTH / (float) sim->WINDOW_HEIGHT, 0.1f, 100.0f, projection);
  shaderSetMatrix(&sim->shader, "projection", projection);
}

void simulationInit(Simulation* sim, const char* configPath)
{
  // OpenGL and GLFW boilerplate
  if (!glfwInit())
  {
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  sim->WINDOW_HEIGHT = 600;
  sim->WINDOW_WIDTH = 800;
  sim->window = glfwCreateWindow(sim->WINDOW_WIDTH, sim->WINDOW_HEIGHT, "ParticleSimulator", NULL, NULL);
  if(!sim->window)
  {
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(sim->window);

  // load OpenGL function pointers
  if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
  {
    return;
  }

  sim->n = 0;

  // parse config file
  srand(time(NULL));

  char* configData = readConfig(configPath);
  if(!configData)
  {
    return;
  }
  cJSON* config = cJSON_Parse(configData);

  const cJSON* gravity = cJSON_GetObjectItemCaseSensitive(config, "gravity");
  if(!cJSON_IsNumber(gravity))
  {
    printf("ERROR::CONFIG::INVALID_GRAVITY: expected float\n");
    return;
  }
  sim->gravity = gravity->valuedouble;

  const cJSON* objects = cJSON_GetObjectItemCaseSensitive(config, "objects");
  if(!cJSON_IsArray(objects))
  {
    printf("ERROR::CONFIG::INVALID_OBJECTS: expected JSON array\n");
    return;
  }

  unsigned int numObjects = cJSON_GetArraySize(objects);
  
  if(numObjects == 0) {
    printf("ERROR::CONFIG::INVALID_OBJECTS: expected non-empty array\n");
    return;
  }

  sim->sphereCount = 0;
  sim->cubeCount = 0;
  sim->pyramidCount = 0;

  // first count the total number of objects for easier memory allocation
  for(int i = 0; i < numObjects; i++) {
    cJSON* object = cJSON_GetArrayItem(objects, i);
    cJSON* type = cJSON_GetObjectItemCaseSensitive(object, "type");
    cJSON* count = cJSON_GetObjectItemCaseSensitive(object, "count");

    if(!cJSON_IsString(type))
    {
      return;
    }

    if(!cJSON_IsNumber(count))
    {
      return;
    }

    if(strcmp(type->valuestring, "sphere") == 0)
    {
      sim->sphereCount += count->valueint;
    }
    else if(strcmp(type->valuestring, "cube") == 0)
    {
      sim->cubeCount += count->valueint;
    }
    else if(strcmp(type->valuestring, "pyramid") == 0)
    {
      sim->pyramidCount += count->valueint;
    }
    else
    {
      printf("ERROR::CONFIG::INVALID_TYPE: expected \"sphere\", \"cube\", or \"pyramid\" for type of object %d instead of %s\n", i + 1, type->valuestring);
      return;
    }
  }

  sim->spheres = malloc(sim->sphereCount * sizeof(Sphere));
  sim->cubes = malloc(sim->cubeCount * sizeof(Cube));
  sim->pyramids = malloc(sim->pyramidCount * sizeof(Pyramid));

  unsigned int sphereIdx = 0;
  unsigned int cubeIdx = 0;
  unsigned int pyramidIdx = 0;

  // process each object description in the array
  for(int i = 0; i < numObjects; i++)
  {
    cJSON* object = cJSON_GetArrayItem(objects, i);
    if(parseObject(sim, object, i, sphereIdx, cubeIdx, pyramidIdx))
    {
      return;
    }

    cJSON* type = cJSON_GetObjectItemCaseSensitive(object, "type");
    cJSON* count = cJSON_GetObjectItemCaseSensitive(object, "count");
    if(strcmp(type->valuestring, "sphere") == 0)
    {
      sphereIdx += count->valueint;
    }
    else if(strcmp(type->valuestring, "cube") == 0)
    {
      cubeIdx += count->valueint;
    }
    else if(strcmp(type->valuestring, "pyramid") == 0)
    {
      pyramidIdx += count->valueint;
    }
  }

  sim->lastTime = 0.0f;
  sim->timeRatio = 0.5f;

  // initialize shader programs
  shaderInit(&sim->shader, "../src/render/shaders/sphere.vs", "../src/render/shaders/sphere.fs");

  shaderInit(&sim->particleShader, "../src/render/shaders/particle.vs", "../src/render/shaders/particle.fs");

  // initialize camera
  cameraInit(&sim->camera, sim->window);
  glEnable(GL_DEPTH_TEST); // draws pixel if it is not behind another pixel
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for(int i = 0; i < sim->sphereCount; i++)
  {
    printf("%f %f\n", sim->spheres[i].radius, sim->spheres[i].mass);
    printf("%f %f %f\n", sim->spheres[i].position[0], sim->spheres[i].position[1], sim->spheres[i].position[2]);
    printf("%d %d\n\n", sim->spheres[i].stacks, sim->spheres[i].sectors);
  }

  sim->n += sim->sphereCount + sim->cubeCount + sim->pyramidCount;

  // initalize vertex data
  sim->sphereVertexCount = sim->sphereCount * sim->spheres[0].n;
  sim->sphereVertices = malloc(sphereVertexCount * sizeof(float));

  for(int i = 0, vertexStart = 0; i < sim->sphereCount; i++, vertexStart += sim->spheres[0].n)
  {
    memcpy(sim->sphereVertices + vertexStart, sphereVertices(sim->spheres + i), sim->spheres[0].n); 
  }

  // set up OpenGL backend for each of the object types
  glGenVertexArrays(1, &sim->sphereVAO);
  glGenBuffers(1, &sim->sphereVBO); 
  
  glBindVertexArray(sim->sphereVAO);

  // store data in buffer
  glBindBuffer(GL_ARRAY_BUFFER, sim->sphereVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sim->sphereVertexCount, sim->sphereVertices, GL_STATIC_DRAW);

  // indicate data layout
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);


  shaderUse(&sim->shader);

  glDrawArrays(GL_TRIANGLES, 0, 6 * s.stacks * s.sectors);

  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void simulationUpdate(Simulation* sim, float deltaTime)
{
  for(int i = 0; i < sim->n; i++)
  {
    sim->objects[i].position[1] -= deltaTime;
  }
}

void simulationRender(Simulation* sim)
{
  // update matrix uniforms
  mat4 model = GLM_MAT4_IDENTITY;
  shaderSetMatrix(&sim->shader, "model", model);

  simulationUpdateCamera(sim);

  for(int i = 0; i < sim->n; i++)
  {
    Object* o = sim->objects + i;
    o->render(sim, o->position, o->orientation, o->color, o->size);
  }
}

void simulationStart(Simulation* sim)
{
  if(sim->n == 0)
  {
    return;
  }

  while(!glfwWindowShouldClose(sim->window))
  {
    if(glfwGetKey(sim->window, GLFW_KEY_ESCAPE))
    {
      glfwSetWindowShouldClose(sim->window, 1);
    }

    cameraKeyboardCallback(&sim->camera, sim->window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update particle positions
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - sim->lastTime;
    simulationUpdate(sim, sim->timeRatio * deltaTime);

    // calculate FPS
    printf("%f\n", 1000.0f / deltaTime);

    sim->lastTime = currentTime;

    simulationRender(sim);

    glfwSwapBuffers(sim->window);

    glfwPollEvents();
  }

  glfwTerminate();
}

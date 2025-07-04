#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

typedef struct Camera {
  // keyboard configuration parameters
  float deltaTime;
  float lastTime;
  float keySensitivity;

  // mouse configuration parameters
  int firstMouse; // whether this is the first time mouse input has been processed
  float mouseSensitivity;
  float fov;
  float lastX;
  float lastY;
  float yaw;
  float pitch;

  vec3 cameraPos; // current position of camera
  vec3 cameraFront; // direction the camera is pointing towards
  vec3 cameraTarget; // the camera's target
  vec3 cameraUp; // up relative to the camera
} Camera;

void cameraInit(Camera* c, GLFWwindow* window);

void cameraProcessKeys(Camera* c, GLFWwindow *window);

void cameraLookAt(Camera* c, mat4 view);

void cameraCustomLookAt(Camera* c, mat4 view);

#endif


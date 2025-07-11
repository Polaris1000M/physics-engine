#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

typedef struct Camera {
  float lastTime; // last time render loop was called

  float keySensitivity; // controls how sensitive movement is during key presses

  int firstCursor;         // whether this is the first time cursor input has been processed
  float cursorSensitivity; // controls how much changes in mouse position rotate view
  float fov;              // visible angle of screen in viewport
  float lastX;            // last x position of cursor
  float lastY;            // last y position of cursor
  float yaw;              // rotation around y axis
  float pitch;            // rotation around x axis

  vec3 cameraPos;    // current position of camera
  vec3 cameraFront;  // direction the camera is pointing towards
  vec3 cameraTarget; // the camera's target
  vec3 cameraUp;     // up relative to the camera
} Camera;

// initializes camera
void cameraInit(Camera* c, GLFWwindow* window);

// processes user input
void cameraKeyboardCallback(Camera* c, GLFWwindow *window);

// compute lookat matrix and stores in view
void cameraLookAt(Camera* c, mat4 view);

// computes lookat matrix with custom mathematics instead of built-in cglm method
void cameraCustomLookAt(Camera* c, mat4 view);

#endif


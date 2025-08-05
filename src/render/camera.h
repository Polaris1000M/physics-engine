/*
 * camera.h
 *
 * Handles generation of view and projection matrices
 * Allows 3D traversal of simulation
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

typedef struct Camera
{
  unsigned int WINDOW_WIDTH;
  unsigned int WINDOW_HEIGHT;

  float lastTime; // last time render loop was called

  float keySensitivity; // controls how sensitive movement is during key presses

  int firstCursor;         // whether this is the first time cursor input has been processed
  float cursorSensitivity; // controls how much changes in mouse position rotate view
  float fov;              // visible angle of screen in viewport
  float lastX;            // last x position of cursor
  float lastY;            // last y position of cursor
  float yaw;              // rotation around y axis
  float pitch;            // rotation around x axis
  float near;             // position of the near plane
  float far;              // position of the far plane

  vec3 cameraPos;    // current position of camera
  vec3 cameraFront;  // direction the camera is pointing towards
  vec3 cameraTarget; // the camera's target
  vec3 cameraUp;     // up relative to the camera

  mat4 projection;
  mat4 view;
  mat4 vp;
} Camera;

// initializes camera
void cameraInit(Camera* c, GLFWwindow* window);

// processes keyboard input
void cameraKeyboardCallback(Camera* c, GLFWwindow *window);

// passed into glfwSetCursorPosCallback for cursor movement callbacks
void cameraCursorCallback(GLFWwindow* window, double xPos, double yPos);

void cameraUpdate(Camera* c);

// computes the coordinates of the 8 corners of the view frustum
void cameraFrustum(Camera* c, vec3* coords);

void cameraPrint(Camera* c);

#endif


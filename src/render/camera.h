/*
 * camera.h
 *
 * Handles generation of view and projection matrices
 * Allows 3D traversal of simulation
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "../physics/object.h"

typedef struct Camera
{
    unsigned int WINDOW_WIDTH;
    unsigned int WINDOW_HEIGHT;

    float lastTime;  // last time render loop was called

    float keySensitivity;  // controls how sensitive movement is during key
                           // presses

    int firstCursor;  // whether this is the first time cursor input has been
                      // processed
    float cursorSensitivity;  // controls how much changes in mouse position
                              // rotate view
    float fov;                // visible angle of screen in viewport
    float lastX;              // last x position of cursor
    float lastY;              // last y position of cursor
    float yaw;                // rotation around y axis
    float pitch;              // rotation around x axis
    float near;               // position of the near plane
    float far;                // position of the far plane

    vec3 cameraPos;     // current position of camera
    vec3 cameraFront;   // direction the camera is pointing towards
    vec3 cameraTarget;  // the camera's target
    vec3 cameraUp;      // up relative to the camera

    mat4 view;        // converts from model space to world space
    mat4 projection;  // converts from world space to view space
    mat4 vp;          // combines view and projection matrices
} Camera;

// initializes camera
void cameraInit(Camera* c, GLFWwindow* window);

// processes keyboard input
void cameraKeyboardCallback(Camera* c, GLFWwindow* window);

// passed into glfwSetCursorPosCallback for cursor movement callbacks
void cameraCursorCallback(GLFWwindow* window, double xPos, double yPos);

// updates the camera's perspective and vp matrices
// should be called after the camera's view direction and position have changed
void cameraUpdate(Camera* c);

// computes the coordinates of the 8 corners of the view frustum
void cameraFrustum(Camera* c, vec3* coords);

// checks whether an object is included inside a camera's frustum for
// frustum occlusion
bool cameraCheckFrustumInclusion(Camera* c, Object* o);

// prints camera data
void cameraPrint(Camera* c);

#endif

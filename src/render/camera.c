#include "camera.h"
#include <cglm/cglm.h>
#include <stdio.h>

// called whenever the cursor position changes
void cameraCursorCallback(GLFWwindow *window, double xPos, double yPos) {
  Camera* c = glfwGetWindowUserPointer(window);

  if(c->firstCursor) {
    c->firstCursor = 0;
    c->lastX = (float) xPos;
    c->lastY = (float) yPos;
  }

  float xOffset = xPos - c->lastX;
  float yOffset = c->lastY - yPos;
  c->lastX = xPos;
  c->lastY = yPos;

  xOffset *= c->cursorSensitivity;
  yOffset *= c->cursorSensitivity;
  
  c->yaw += xOffset;
  c->pitch += yOffset;

  vec3 direction = {cos(glm_rad(c->yaw)) * cos(glm_rad(c->pitch)), sin(glm_rad(c->pitch)), sin(glm_rad(c->yaw)) * cos(glm_rad(c->pitch))};
  glm_normalize_to(direction, c->cameraFront);
}

// called whenever scroll input is received
void cameraScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
  Camera* c = glfwGetWindowUserPointer(window);

  c->fov -= (float) yOffset;
  if(c->fov < 1.0f) {
    c->fov = 1.0f;
  }

  if(c->fov > 45.0f) {
    c->fov = 45.0f;
  }
}

void cameraInit(Camera* c, GLFWwindow* window) {
  c->cameraPos[0] = 0.0f;
  c->cameraPos[1] = 0.0f;
  c->cameraPos[2] = 3.0f; // set camera along positive z axis

  c->cameraFront[0] = 0.0f;
  c->cameraFront[1] = 0.0f;
  c->cameraFront[2] = -1.0f; // camera points towards negative z axis

  c->cameraUp[0] = 0.0f;
  c->cameraUp[1] = 1.0f;
  c->cameraUp[2] = 0.0f;

  c->lastTime = 0.0f;

  // keyboard configuration
  c->keySensitivity = 2.5f; 

  // mouse configuration
  c->firstCursor = 1;
  c->cursorSensitivity = 0.1f;
  c->fov = 45.0f;
  c->lastX = 400.0f;
  c->lastY = 300.0f;
  c->yaw = -90.0f;
  c->pitch = 0.0f;
  
  glfwSetWindowUserPointer(window, c);                         // allows callbacks to access camera struct
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // sets window input to the cursor
  glfwSetCursorPosCallback(window, cameraCursorCallback);          // calls function whenever cursor position changes
  glfwSetScrollCallback(window, cameraScrollCallback);         // called whenever camera scrolls
}

void cameraKeyboardCallback(Camera* c, GLFWwindow* window) {
  float deltaTime = glfwGetTime() - c->lastTime;
  c->lastTime = glfwGetTime();

  const float cameraSpeed = deltaTime * c->keySensitivity;

  if(glfwGetKey(window, GLFW_KEY_W)) {
    vec3 mov;
    glm_vec3_scale(c->cameraFront, cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_S)) {
    vec3 mov;
    glm_vec3_scale(c->cameraFront, -cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_A)) {
    vec3 mov;
    glm_vec3_crossn(c->cameraFront, c->cameraUp, mov);
    glm_vec3_scale(mov, -cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_D)) {
    vec3 mov;
    glm_vec3_crossn(c->cameraFront, c->cameraUp, mov);
    glm_vec3_scale(mov, cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_SPACE)) {
    vec3 mov;
    glm_vec3_scale(c->cameraUp, cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
    vec3 mov;
    glm_vec3_scale(c->cameraUp, -cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }
}

void cameraLookAt(Camera* c, mat4 view) {
  vec3 cameraTarget;
  glm_vec3_add(c->cameraPos, c->cameraFront, cameraTarget);
  glm_lookat(c->cameraPos, cameraTarget, c->cameraUp, view);
}

void cameraCustomLookAt(Camera* c, mat4 view) {
  vec3 cameraDirection;
  glm_vec3_negate_to(c->cameraFront, cameraDirection);

  vec3 cameraRight;
  glm_vec3_crossn(c->cameraUp, cameraDirection, cameraRight);

  vec3 cameraUp;
  glm_vec3_cross(cameraDirection, cameraRight, cameraUp);

  mat4 translate = GLM_MAT4_IDENTITY;
  vec3 negatePos;
  glm_vec3_negate_to(c->cameraPos, negatePos);
  glm_translate_make(translate, negatePos);
  mat4 basis = {
    {cameraRight[0], cameraUp[0], cameraDirection[0], 0.0f},
    {cameraRight[1], cameraUp[1], cameraDirection[1], 0.0f},
    {cameraRight[2], cameraUp[2], cameraDirection[2], 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f}
  };
  glm_mat4_mul(basis, translate, view);
}

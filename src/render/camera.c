#include "camera.h"
#include <cglm/cglm.h>
#include <stdio.h>

// called whenever the cursor position changes
void cameraCursorCallback(GLFWwindow *window, double xPos, double yPos)
{
  Camera* c = glfwGetWindowUserPointer(window);

  if(c->firstCursor)
  {
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

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    Camera* c = glfwGetWindowUserPointer(window);
    c->WINDOW_HEIGHT = height;
    c->WINDOW_WIDTH = width;
}

// called whenever scroll input is received
void cameraScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
  Camera* c = glfwGetWindowUserPointer(window);

  c->fov -= (float) yOffset;
  if(c->fov < 1.0f)
  {
    c->fov = 1.0f;
  }

  if(c->fov > 45.0f)
  {
    c->fov = 45.0f;
  }
}

void cameraInit(Camera* c, GLFWwindow* window)
{
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  c->WINDOW_WIDTH = width; 
  c->WINDOW_HEIGHT = height;

  glm_vec3_copy((vec3) {0.0f, 2.0f, 9.0f}, c->cameraPos);
  glm_vec3_copy((vec3) {0.0f, 0.0f, -1.0f}, c->cameraFront);
  glm_vec3_copy((vec3) {0.0f, 1.0f, 0.0f}, c->cameraUp);

  c->lastTime = 0.0f;

  // keyboard configuration
  c->keySensitivity = 20.0f; 

  // mouse configuration
  c->firstCursor = 1;
  c->cursorSensitivity = 0.1f;
  c->fov = 45.0f;
  c->lastX = 400.0f;
  c->lastY = 300.0f;
  c->yaw = -90.0f;
  c->pitch = 0.0f;
  c->near = 0.1f;
  c->far = 50.0f;
  
  glfwSetWindowUserPointer(window, c);                         // allows callbacks to access camera struct
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // sets window input to the cursor
  glfwSetCursorPosCallback(window, cameraCursorCallback);          // calls function whenever cursor position changes
  glfwSetScrollCallback(window, cameraScrollCallback);         // called whenever camera scrolls
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void cameraKeyboardCallback(Camera* c, GLFWwindow* window)
{
  float deltaTime = glfwGetTime() - c->lastTime;
  c->lastTime = glfwGetTime();

  const float cameraSpeed = deltaTime * c->keySensitivity;

  if(glfwGetKey(window, GLFW_KEY_W))
  {
    vec3 mov;
    glm_vec3_scale(c->cameraFront, cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_S)) {
    vec3 mov;
    glm_vec3_scale(c->cameraFront, -cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_A))
  {
    vec3 mov;
    glm_vec3_crossn(c->cameraFront, c->cameraUp, mov);
    glm_vec3_scale(mov, -cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_D))
  {
    vec3 mov;
    glm_vec3_crossn(c->cameraFront, c->cameraUp, mov);
    glm_vec3_scale(mov, cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_SPACE))
  {
    vec3 mov;
    glm_vec3_scale(c->cameraUp, cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
  {
    vec3 mov;
    glm_vec3_scale(c->cameraUp, -cameraSpeed, mov);
    glm_vec3_add(c->cameraPos, mov, c->cameraPos);
  }

  if(c->cameraPos[1] < 0.2f)
  {
    c->cameraPos[1] = 0.2f;
  }
}

void cameraUpdate(Camera* c)
{
  vec3 cameraTarget;
  glm_vec3_add(c->cameraPos, c->cameraFront, cameraTarget);
  glm_lookat(c->cameraPos, cameraTarget, c->cameraUp, c->view);

  glm_perspective(glm_rad(c->fov), (float) c->WINDOW_WIDTH / (float) c->WINDOW_HEIGHT, c->near, c->far, c->projection);

  glm_mat4_mul(c->projection, c->view, c->vp);
}

void cameraFrustum(Camera* c, vec3* corners)
{
  mat4 inv;
  glm_mat4_inv(c->vp, inv);

  int signs[2] = {-1, 1};

  // compute near and far plane values
  for(unsigned int x = 0; x < 2; x++)
  {
    for(unsigned int y = 0; y < 2; y++)
    {
      for(unsigned int z = 0; z < 2; z++)
      {
        vec4 corner = {signs[x], signs[y], signs[z], 1.0f};
        glm_mat4_mulv(inv, corner, corner);

        for(unsigned int i = 0; i < 3; i++)
        {
          corners[4 * x + 2 * y + z][i] = corner[i] / corner[3];
        }
      }
    }
  }
}

void cameraPrint(Camera* c)
{
  printf("CAMERA\n");
  printf("fov: %f\n", c->fov);
  printf("aspect: %f\n", (float) c->WINDOW_WIDTH / (float) c->WINDOW_HEIGHT);
  printf("near: %f\n", (float) c->near);
  printf("far: %f\n", (float) c->far);
  printf("cameraPos: ");
  glm_vec3_print(c->cameraPos, stdout);
  printf("cameraFront: ");
  glm_vec3_print(c->cameraFront, stdout);
  printf("view: ");
  glm_mat4_print(c->view, stdout);
  printf("projection: ");
  glm_mat4_print(c->projection, stdout);
}

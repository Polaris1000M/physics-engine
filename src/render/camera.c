#include "camera.h"

#include <stdio.h>

void cameraInit(Camera* c, GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    c->WINDOW_WIDTH = width;
    c->WINDOW_HEIGHT = height;

    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, c->cameraUp);

    // begin with camera navigation enabled
    c->enabled = 1;

    c->lastTime = 0.0f;

    // keyboard configuration
    c->keySensitivity = 20.0f;

    // mouse configuration
    c->firstCursor = 1;
    c->cursorSensitivity = 0.1f;
    c->fov = 45.0f;
    c->lastX = width / 2;
    c->lastY = height / 2;
    c->near = 0.1f;
    c->far = 100.0f;

    if (fabsf(c->cameraFront[1]) > 0.9999f)
    {
        if (c->cameraFront[1] > 0.0f)
        {
            c->cameraFront[1] = 0.99f;
        }
        else
        {
            c->cameraFront[1] = -0.99f;
        }
        c->cameraFront[2] = -0.01f;
    }

    float x = c->cameraFront[0];
    float y = c->cameraFront[1];
    float z = c->cameraFront[2];
    c->pitch = atan2(y, sqrt(x * x + z * z)) * 180.0f / M_PI;
    if (fabsf(c->pitch) >= 90.0f)
    {
        if (c->pitch < 0.0f)
        {
            c->pitch = -180.0f - c->pitch;
        }
        else
        {
            c->pitch = 180.0f - c->pitch;
        }
    }

    c->yaw = atan2(z, x) * 180.0f / M_PI;
    if (c->yaw < 0.0f)
    {
        c->yaw += 360.0f;
    }
}

void cameraUpdate(Camera* c)
{
    vec3 cameraTarget;
    glm_vec3_add(c->cameraPos, c->cameraFront, cameraTarget);
    glm_lookat(c->cameraPos, cameraTarget, c->cameraUp, c->view);

    glm_perspective(glm_rad(c->fov),
                    (float)c->WINDOW_WIDTH / (float)c->WINDOW_HEIGHT, c->near,
                    c->far, c->projection);

    glm_mat4_mul(c->projection, c->view, c->vp);
}

void cameraFrustum(Camera* c, vec3* corners)
{
    mat4 inv;
    glm_mat4_inv(c->vp, inv);

    int signs[2] = {-1, 1};

    // compute near and far plane values
    for (unsigned int x = 0; x < 2; x++)
    {
        for (unsigned int y = 0; y < 2; y++)
        {
            for (unsigned int z = 0; z < 2; z++)
            {
                vec4 corner = {signs[x], signs[y], signs[z], 1.0f};
                glm_mat4_mulv(inv, corner, corner);

                for (unsigned int i = 0; i < 3; i++)
                {
                    corners[4 * x + 2 * y + z][i] = corner[i] / corner[3];
                }
            }
        }
    }
}

// handles movement
void cameraProcessInput(Camera* c, GLFWwindow* window)
{
    float deltaTime = glfwGetTime() - c->lastTime;
    c->lastTime = glfwGetTime();

    if (!c->enabled)
    {
        return;
    }

    const float cameraSpeed = deltaTime * c->keySensitivity;

    if (glfwGetKey(window, GLFW_KEY_W))
    {
        vec3 mov;
        glm_vec3_copy(c->cameraFront, mov);
        mov[1] = 0;
        glm_vec3_scale_as(mov, cameraSpeed, mov);
        glm_vec3_add(c->cameraPos, mov, c->cameraPos);
    }

    if (glfwGetKey(window, GLFW_KEY_S))
    {
        vec3 mov;
        glm_vec3_copy(c->cameraFront, mov);
        mov[1] = 0;
        glm_vec3_scale_as(mov, -cameraSpeed, mov);
        glm_vec3_add(c->cameraPos, mov, c->cameraPos);
    }

    if (glfwGetKey(window, GLFW_KEY_A))
    {
        vec3 mov;
        glm_vec3_crossn(c->cameraFront, c->cameraUp, mov);
        glm_vec3_scale(mov, -cameraSpeed, mov);
        glm_vec3_add(c->cameraPos, mov, c->cameraPos);
    }

    if (glfwGetKey(window, GLFW_KEY_D))
    {
        vec3 mov;
        glm_vec3_crossn(c->cameraFront, c->cameraUp, mov);
        glm_vec3_scale(mov, cameraSpeed, mov);
        glm_vec3_add(c->cameraPos, mov, c->cameraPos);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE))
    {
        vec3 mov;
        glm_vec3_scale(c->cameraUp, cameraSpeed, mov);
        glm_vec3_add(c->cameraPos, mov, c->cameraPos);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
    {
        vec3 mov;
        glm_vec3_scale(c->cameraUp, -cameraSpeed, mov);
        glm_vec3_add(c->cameraPos, mov, c->cameraPos);
    }
}

void cameraPrint(Camera* c)
{
    printf("CAMERA\n");
    printf("keySensitivity: %f\n", c->keySensitivity);
    printf("fov: %f\n", c->fov);
    printf("aspect: %f\n", (float)c->WINDOW_WIDTH / (float)c->WINDOW_HEIGHT);
    printf("near: %f\n", (float)c->near);
    printf("far: %f\n", (float)c->far);
    printf("yaw: %f\n", (float)c->yaw);
    printf("pitch: %f\n", (float)c->pitch);
    printf("cameraPos: ");
    glm_vec3_print(c->cameraPos, stdout);
    printf("cameraFront: ");
    glm_vec3_print(c->cameraFront, stdout);
    printf("view: ");
    glm_mat4_print(c->view, stdout);
    printf("projection: ");
    glm_mat4_print(c->projection, stdout);
}


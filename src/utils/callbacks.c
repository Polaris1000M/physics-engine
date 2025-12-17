#include "callbacks.h"

// called whenever the cursor position changes
void cameraCursorCallback(GLFWwindow* window, double xPos, double yPos)
{
    Camera* c = &((Simulation*) glfwGetWindowUserPointer(window))->camera;

    if (c->firstCursor)
    {
        c->firstCursor = 0;
        c->lastX = (float)xPos;
        c->lastY = (float)yPos;
    }

    float xOffset = xPos - c->lastX;
    float yOffset = c->lastY - yPos;
    c->lastX = xPos;
    c->lastY = yPos;

    if (!c->enabled || !c->focused)
    {
        return;
    }

    xOffset *= c->cursorSensitivity;
    yOffset *= c->cursorSensitivity;

    c->yaw += xOffset;
    c->pitch += yOffset;
    if (c->pitch < -89.0f)
    {
        c->pitch = -89.0f;
    }
    else if (c->pitch > 89.0f)
    {
        c->pitch = 89.0f;
    }
    
    vec3 direction = {cos(glm_rad(c->yaw)) * cos(glm_rad(c->pitch)),
                      sin(glm_rad(c->pitch)),
                      sin(glm_rad(c->yaw)) * cos(glm_rad(c->pitch))};
    glm_normalize_to(direction, c->cameraFront);
}

// called whenever scroll input is received
// changes FOV based on scrolling
void cameraScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    Camera* c = &((Simulation*) glfwGetWindowUserPointer(window))->camera;

    c->fov -= (float)yOffset;
    if (c->fov < 1.0f)
    {
        c->fov = 1.0f;
    }

    if (c->fov > 45.0f)
    {
        c->fov = 45.0f;
    }
}

// sets all the necessary callbacks for the camera to function
void cameraEnableNavigation(Camera* c, GLFWwindow* window)
{
    c->enabled = 1;
    glfwSetInputMode(window, GLFW_CURSOR,
                 GLFW_CURSOR_DISABLED);  // sets window input to the cursor
    glfwSetScrollCallback(
        window, cameraScrollCallback);  // called whenever camera scrolls
}

// disables all callbacks for camera navigation
void cameraDisableNavigation(Camera* c, GLFWwindow* window)
{
    c->enabled = 0;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetScrollCallback(window, NULL);
}

// disables camera callbacks whenever window is no longer in focus
void cameraFocusCallback(GLFWwindow* window, int focused)
{
    Camera* c = &((Simulation*) glfwGetWindowUserPointer(window))->camera;
    if (!c->enabled)
    {
        return;
    }

    if (focused)
    {
        c->focused = 1;
        cameraEnableNavigation(c, window);
    }
    else
    {
        c->focused = 0;
        cameraDisableNavigation(c, window);
    }
}

void cameraToggleNavigation(Camera *c, GLFWwindow *window)
{
    if (c->enabled)
    {
        cameraDisableNavigation(c, window);
    }
    else
    {
        cameraEnableNavigation(c, window);
    }
}

// adjusts to changes in the framebuffer's size
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    Simulation* sim = glfwGetWindowUserPointer(window);
    sim->camera.WINDOW_HEIGHT = height;
    sim->camera.WINDOW_WIDTH = width;

    simulationUpdate(sim);
    simulationRender(sim);

    glfwSwapBuffers(sim->window);
    glfwPollEvents();
}

void simulationKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        Camera* c = &((Simulation*) glfwGetWindowUserPointer(window))->camera;
        cameraToggleNavigation(c, window);
    }
}

void callbacksInit(Simulation* sim)
{
    glfwSetKeyCallback(sim->window, simulationKeyCallback);
    glfwSetWindowUserPointer(sim->window, sim);  // allows callbacks to access simulation struct
    glfwSetWindowFocusCallback(sim->window, cameraFocusCallback);
    glfwSetFramebufferSizeCallback(sim->window, framebufferSizeCallback);
    glfwSetCursorPosCallback(sim->window,
                             cameraCursorCallback);  // calls function whenever
                                                     // cursor position changes
}


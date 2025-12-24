/*
 * render.h
 *
 * Manages updating all rendering and OpenGL related components of the simulation (e.g., camera, shadow, shaders, text) and executes rendering
 */

#ifndef RENDER_H
#define RENDER_H

#include <glad/glad.h>  // must be included first
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

typedef struct Simulation Simulation;

unsigned int renderInit(Simulation* sim);

void renderUpdate(Simulation* sim);

#endif


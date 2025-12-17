/*
 * text.h
 *
 * Helps render strings to OpenGL window with FreeType
 */

#ifndef TEXT_H
#define TEXT_H

#include <cglm/cglm.h>
#include "shader.h"

typedef struct Character
{
    unsigned int ID; // character's texture ID
    vec2 size;
    vec2 bearing;
    unsigned int advance;
} Character;

typedef struct Text
{
    Character characters[128];
    Shader shader;
    mat4 projection;
    unsigned int VAO, VBO;
} Text;

int textInit(Text* t, const char* fontPath);

void textRender(Text* t, const char* text, int width, int height, float x, float y, float scale, vec3 color);

#endif

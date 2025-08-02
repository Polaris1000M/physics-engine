/*
 * shader.h
 *
 * Helper struct which manages compiling a shader program and setting uniforms
 */

#ifndef SHADER_H
#define SHADER_H

#include <cglm/cglm.h>

typedef struct Shader
{
  unsigned int ID;
} Shader;

// initializes shader
void shaderInit(Shader* s, const char* vertexPath, const char* fragmentPath);

// uses shader program
void shaderUse(Shader* s);

// sets an integer uniform (1i)
void shaderSetInt(Shader* s, const char* name, int value);

// sets float uniform (1f)
void shaderSetFloat(Shader* s, const char* name, float value);

// sets 4x4 matrix uniform (4fv)
void shaderSetMatrix(Shader* s, const char* name, mat4 mat);

// sets 3-dim vector uniform (3f)
void shaderSetVector(Shader* s, const char* name, vec3 vec);

#endif

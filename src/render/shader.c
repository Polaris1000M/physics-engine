#include "shader.h"
#include "../utils/parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <cglm/cglm.h>

void shaderInit(Shader* s, const char* vertexPath, const char* fragmentPath)
{
    // read vertex shader source
    char* vertexShaderSource = parseFile(vertexPath, "SHADER");
    if(!vertexShaderSource)
    {
        return;
    }

    // read fragment shader source
    char* fragmentShaderSource = parseFile(fragmentPath, "SHADER");
    if(!fragmentShaderSource)
    {
        return;
    }

    // compile shader programs
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char**) &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
    }

    unsigned int fragmentShader= glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char**) &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
    }

    // attach and link shaders into shader program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
    }

    // clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free(vertexShaderSource);
    free(fragmentShaderSource);

    s->ID = shaderProgram;
}

void shaderUse(Shader* s)
{
    // adds the program to the current rendering state
    glUseProgram(s->ID);
}

void shaderSetInt(Shader* s, const char* name, int value)
{
    glUniform1i(glGetUniformLocation(s->ID, name), value); 
}

void shaderSetFloat(Shader* s, const char* name, float value)
{
    glUniform1f(glGetUniformLocation(s->ID, name), value); 
}

void shaderSetMatrix(Shader* s, const char* name, mat4 mat)
{
    glUniformMatrix4fv(glGetUniformLocation(s->ID, name), 1, GL_FALSE, (const float*) mat);
}

void shaderSetVector(Shader* s, const char* name, vec3 vec)
{
    glUniform3f(glGetUniformLocation(s->ID, name), vec[0], vec[1], vec[2]);
}

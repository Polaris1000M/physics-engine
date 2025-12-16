#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION // modifies stb_image.h to only include relevant source code definitions
#include "stb_image.h"
#include <stdio.h>
#include <glad/glad.h>

void textureImageInit(Texture* t, unsigned int textureUnit, const char* textureSource)
{
    int width;
    int height;
    int nrChannels;

    unsigned char* data = stbi_load(textureSource, &width, &height, &nrChannels, 0);

    if(!data)
    {
        printf("ERROR::TEXTURE::INITIALIZATION_ERROR: failed to load texture: %s\n", textureSource);
        return;
    }

    unsigned int texture;
    glGenTextures(1, &texture);

    // defines which texture unit (i.e., one of multiple slots on GPU for shader program for textures) will be used
    glActiveTexture(textureUnit);

    glBindTexture(GL_TEXTURE_2D, texture);

    // set texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    t->ID = texture;
    t->textureUnit = textureUnit;
}

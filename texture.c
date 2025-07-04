#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION // modifies stb_image.h to only include relevant source code definitions
#include "stb_image.h"
#include <stdio.h>
#include <glad/glad.h>

void textureInit(Texture* t, unsigned int textureUnit, const char* textureSource, int flip, int transparent) {
  int width;
  int height;
  int nrChannels;

  stbi_set_flip_vertically_on_load(flip);

  unsigned char* data = stbi_load(textureSource, &width, &height, &nrChannels, 0);

  if(!data) {
    printf("Failed to load texture: %s\n", textureSource);
    return;
  }

  unsigned int texture;
  glGenTextures(1, &texture);

  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, texture);

  // set texture wrapping and filtering options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if(transparent) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  }
  else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  }
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  t->ID = texture;
  t->textureUnit = textureUnit;
}

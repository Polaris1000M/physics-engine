#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct Texture {
  unsigned int ID;
  unsigned int textureUnit;
} Texture;

void textureInit(Texture* t, unsigned int textureUnit, const char* textureSource, int flip, int transparent);

#endif

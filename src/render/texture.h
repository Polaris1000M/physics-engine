#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct Texture
{
    unsigned int ID;
    unsigned int textureUnit; // location of texture on GPU
} Texture;

// initializes texture with a provided texture and source path
void textureImageInit(Texture* t, unsigned int textureUnit, const char* textureSource);

// void textureGlyphInit(Texture* t,)

#endif

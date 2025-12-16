/*
 * text.h
 *
 * Helps render strings to OpenGL window with FreeType
 */

#ifndef TEXT_H
#define TEXT_H

typedef struct Glyph
{

} Glyph;

typedef struct Text
{
    Glyph alphabet[128];

} Text;

int textInit(Text* t);

#endif

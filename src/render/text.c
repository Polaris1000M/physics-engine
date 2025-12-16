#include "text.h"

#include <ft2build.h>
#include FT_FREETYPE_H

int textInit(Text* t)
{
    FT_Library ft;

    if (FT_Init_FreeType(&ft))
    {
        printf("%s", "ERROR::TEXT: could not initialize FreeType library\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "../fonts/arial.ttf", 0, &face))
    {
        printf("%s", "ERROR::TEXT: failed to load font\n");
        return 1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        printf("%s", "ERROR:TEXT: failed to load glyph\n");
        return 1;
    }

    return 0;
}

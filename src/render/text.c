#include "text.h"

#include <ft2build.h>
#include <glad/glad.h>
#include FT_FREETYPE_H

int textInit(Text* t, const char* fontPath)
{
    FT_Library ft;

    if (FT_Init_FreeType(&ft))
    {
        printf("%s",
               "ERROR::FREETYPE: could not initialize FreeType library\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face))
    {
        printf("%s", "ERROR::FREETYPE: failed to load font\n");
        return 1;
    }

    t->pixelSizes = 48;
    t->gapBetweenLines = (t->pixelSizes + 3) / 3;

    FT_Set_Pixel_Sizes(face, 0, t->pixelSizes);
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL))
    {
        printf("ERROR::FREETYPE: Failed to load glyph\n");
        return 1;
    }

    // initialize 128 ASCII characters
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printf("ERROR::FREETYPE: Failed to load glyph\n");
            continue;
        }

        unsigned int ID;
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                     face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        vec2 size = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
        vec2 bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
        t->characters[c].ID = ID;
        glm_vec2_copy(size, t->characters[c].size);
        glm_vec2_copy(bearing, t->characters[c].bearing);
        t->characters[c].advance = (unsigned int)(face->glyph->advance.x);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // initialize shader
    shaderInit(&t->shader, "../src/render/shaders/font-vs.glsl",
               "../src/render/shaders/font-fs.glsl");

    // bind buffers
    glGenVertexArrays(1, &t->VAO);
    glGenBuffers(1, &t->VBO);
    glBindVertexArray(t->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return 0;
}

void textRender(Text* t, int lines, char** text, int width, int height, float x,
                float y, float scale, vec3 color)
{
    shaderUse(&t->shader);
    glm_ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f,
              t->projection);
    shaderSetMatrix(&t->shader, "projection", t->projection);

    glUniform3f(glGetUniformLocation(t->shader.ID, "textColor"), color[0],
                color[1], color[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(t->VAO);

    float origX = x;

    for (int i = lines - 1; i >= 0; i--)
    {
        int N = strlen(text[i]);
        float dy = 0;
        for (int j = 0; j < N; j++)
        {
            Character ch = t->characters[text[i][j]];

            // advance cursor on space character
            if (text[i][j] == ' ')
            {
                x += (ch.advance >> 6) * scale;
                continue;
            }

            float xpos = x + ch.bearing[0] * scale;
            float ypos = y - (ch.size[1] - ch.bearing[1]) * scale;

            float w = ch.size[0] * scale;
            float h = ch.size[1] * scale;

            // get maximum height among current characters
            if (h > dy)
            {
                dy = h;
            }

            float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 1.0f},

                {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}};

            glBindTexture(GL_TEXTURE_2D, ch.ID);
            glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.advance >> 6) * scale;
        }

        y += dy + t->gapBetweenLines;
        x = origX;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


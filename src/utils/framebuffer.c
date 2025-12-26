#include "framebuffer.h"

#include <glad/glad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void saveFramebuffer(unsigned int FBO, unsigned int width, unsigned int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    float* data = malloc(width * height * sizeof(float));
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);

    // Create flipped data
    unsigned char* flipped = malloc(width * height);

    for (unsigned int y = 0; y < height; y++)
    {
        for (unsigned int x = 0; x < width; x++)
        {
            unsigned int srcIdx = (height - 1 - y) * width + x;
            unsigned int dstIdx = y * width + x;

            float depth = data[srcIdx];
            depth = (depth < 0.0f) ? 0.0f : (depth > 1.0f) ? 1.0f : depth;
            flipped[dstIdx] = (unsigned char)(depth * 255.0f);
        }
    }

    stbi_write_bmp("fb.bmp", width, height, 1, flipped);

    free(data);
    free(flipped);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


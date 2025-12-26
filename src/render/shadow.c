#include <glad/glad.h>  // must be included first

#include "shadow.h"

unsigned int shadowInit(Shadow* s, Camera* c, vec3 lightDir)
{
    s->dist = c->far - c->near;
    s->padding = 15.0f;
    glm_vec3_copy(lightDir, s->lightDir);
    glm_vec3_scale(s->lightDir, -s->dist, s->lightDelta);
    s->SHADOW_WIDTH = 8192;
    s->SHADOW_HEIGHT = 8192;
    shaderInit(&s->shader, "../src/render/shaders/shadow-vs.glsl",
               "../src/render/shaders/shadow-fs.glsl");

    glGenFramebuffers(1, &s->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s->FBO);

    glGenTextures(1, &s->depthMap);
    glBindTexture(GL_TEXTURE_2D, s->depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, s->SHADOW_WIDTH,
                 s->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                    GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           s->depthMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf(
            "ERROR::SHADOW::INCOMPLETE_FRAMEBUFFER: framebuffer initialization "
            "failed\n");
        return 1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

void shadowUpdate(Shadow* s, Camera* c)
{
    // compute frustum center
    cameraFrustum(c, s->corners);
    glm_vec3_copy(GLM_VEC3_ZERO, s->frustumCenter);
    for (int i = 0; i < 8; i++)
    {
        glm_vec3_add(s->corners[i], s->frustumCenter, s->frustumCenter);
    }
    glm_vec3_scale(s->frustumCenter, 1.0f / 8.0f, s->frustumCenter);

    // compute light position
    glm_vec3_copy(s->frustumCenter, s->lightPos);
    glm_vec3_add(s->lightDelta, s->lightPos, s->lightPos);

    // create view matrix
    if (fabsf(s->lightDir[1]) > 0.99)
    {
        glm_lookat(s->lightPos, s->frustumCenter, (vec3){0.0f, 0.0f, -1.0f},
                   s->view);
    }
    else
    {
        glm_lookat(s->lightPos, s->frustumCenter, (vec3){0.0f, 1.0f, 0.0f},
                   s->view);
    }

    // create orthographic projection matrix

    // find minimum and maximum coordinates for each frustum corner
    vec3 curMin;
    vec3 curMax;
    for (int i = 0; i < 8; i++)
    {
        glm_mat4_mulv3(s->view, s->corners[i], 1.0f, s->corners[i]);

        if (i == 0)
        {
            glm_vec3_copy(s->corners[i], curMin);
            glm_vec3_copy(s->corners[i], curMax);
        }
        else
        {
            glm_vec3_minv(s->corners[i], curMin, curMin);
            glm_vec3_maxv(s->corners[i], curMax, curMax);
        }
    }

    // only generate a new shadow map if the furstum has exited the prior range
    unsigned int replace = 0;
    for (int i = 0; i < 3; i++)
    {
        if (curMin[i] < s->min[i] || curMax[i] > s->max[i])
        {
            replace = 1;
            break;
        }
    }

    if (replace)
    {
        glm_vec3_copy(curMin, s->min);
        glm_vec3_copy(curMax, s->max);
        for (int i = 0; i < 3; i++)
        {
            s->min[i] -= s->padding;
            s->max[i] += s->padding;
        }
        glm_ortho(s->min[0], s->max[0], s->min[1], s->max[1], -s->max[2],
                  -s->min[2], s->projection);
    }

    // create vp matrix
    glm_mat4_mul(s->projection, s->view, s->vp);
}

void shadowPrint(Shadow* s)
{
    printf("SHADOW\n");
    printf("dist: %f\n", s->dist);
    printf("lightDir: ");
    glm_vec3_print(s->lightDir, stdout);
    printf("lightPos: ");
    glm_vec3_print(s->lightPos, stdout);
    printf("lightDelta: ");
    glm_vec3_print(s->lightDelta, stdout);
    printf("frustumCenter: ");
    glm_vec3_print(s->frustumCenter, stdout);
    printf("min: ");
    glm_vec3_print(s->min, stdout);
    printf("max: ");
    glm_vec3_print(s->max, stdout);
    printf("view: ");
    glm_mat4_print(s->view, stdout);
    printf("projection: ");
    glm_mat4_print(s->projection, stdout);
    printf("corners: \n");
    for (int i = 0; i < 8; i++)
    {
        glm_vec3_print(s->corners[i], stdout);
    }
}


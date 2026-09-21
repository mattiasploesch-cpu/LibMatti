// Port of net.neoforged.fml.earlydisplay.render.EarlyFramebuffer (implementation).
// Every GL call routes through the GlStateManager / GL binding layer like the
// rest of the port. The blit is the heart of the class: the source rectangle is
// vertically flipped (srcY0 = height, srcY1 = 0) because the y-down screen-pixel
// projection of the element shaders lands bottom-up in the FBO - the flipped
// read-back mirrors it upright onto the window.

#include "libmatti/net/neoforged/fml/earlydisplay/EarlyFramebuffer.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdlib.h>

// Java: RenderElement.clamp(int, int, int)
static int clamp(int v, int min, int max)
{
    return v < min ? min : (v > max ? max : v);
}

struct LIBMATTI_FML_EarlyFramebuffer
{
    unsigned int framebuffer;
    unsigned int texture;
    int width;
    int height;
};

// Java: new EarlyFramebuffer(width, height) - the constructor body
LIBMATTI_FML_EarlyFramebuffer *LIBMATTI_FML_EarlyFramebuffer_New(int width, int height)
{
    LIBMATTI_FML_EarlyFramebuffer *fb = calloc(1, sizeof(LIBMATTI_FML_EarlyFramebuffer));
    fb->width = width;
    fb->height = height;
    fb->framebuffer = (unsigned int) LIBMATTI_B3D_GlStateManager_GenFramebuffers();
    fb->texture = 0;
    LIBMATTI_GL_glGenTextures(1, &fb->texture);

    LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_FRAMEBUFFER, fb->framebuffer);
    LIBMATTI_B3D_GlStateManager_BindTexture((int) fb->texture);
    LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA, width, height, 0,
                             LIBMATTI_GL_GL_RGBA, LIBMATTI_GL_GL_UNSIGNED_BYTE, NULL);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER, LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER, LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glFramebufferTexture2D(LIBMATTI_GL_GL_FRAMEBUFFER, LIBMATTI_GL_GL_COLOR_ATTACHMENT0,
                                       LIBMATTI_GL_GL_TEXTURE_2D, fb->texture, 0);
    LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_FRAMEBUFFER, 0);
    return fb;
}

// Java: public void resize(int width, int height)
void LIBMATTI_FML_EarlyFramebuffer_Resize(LIBMATTI_FML_EarlyFramebuffer *fb, int width, int height)
{
    if (fb->width != width || fb->height != height)
    {
        LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_FRAMEBUFFER, fb->framebuffer);
        LIBMATTI_B3D_GlStateManager_BindTexture((int) fb->texture);
        fb->width = width;
        fb->height = height;
        LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA, width, height, 0,
                                 LIBMATTI_GL_GL_RGBA, LIBMATTI_GL_GL_UNSIGNED_BYTE, NULL);
    }
}

// Java: public void activate()
void LIBMATTI_FML_EarlyFramebuffer_Activate(LIBMATTI_FML_EarlyFramebuffer *fb)
{
    LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_FRAMEBUFFER, fb->framebuffer);
}

// Java: public void deactivate()
void LIBMATTI_FML_EarlyFramebuffer_Deactivate(LIBMATTI_FML_EarlyFramebuffer *fb)
{
    (void) fb;
    LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_FRAMEBUFFER, 0);
}

// Java: public void blitToScreen(ThemeColor backgroundColor, int windowFBWidth, int windowFBHeight)
void LIBMATTI_FML_EarlyFramebuffer_BlitToScreen(LIBMATTI_FML_EarlyFramebuffer *fb, float backgroundR,
                                                float backgroundG, float backgroundB, int windowFBWidth,
                                                int windowFBHeight)
{
    float wscale = (float) windowFBWidth / fb->width;
    float hscale = (float) windowFBHeight / fb->height;
    float scale = (wscale < hscale ? wscale : hscale) / 2.0f;
    int wleft = (int) ((float) windowFBWidth * 0.5f - scale * fb->width);
    int wtop = (int) ((float) windowFBHeight * 0.5f - scale * fb->height);
    int wright = (int) ((float) windowFBWidth * 0.5f + scale * fb->width);
    int wbottom = (int) ((float) windowFBHeight * 0.5f + scale * fb->height);

    // Java: bindDrawFramebuffer(0); bindReadFramebuffer(this.framebuffer);
    LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_DRAW_FRAMEBUFFER, 0);
    LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_READ_FRAMEBUFFER, fb->framebuffer);

    LIBMATTI_B3D_GlStateManager_ClearColor(backgroundR, backgroundG, backgroundB, 1.0f);
    LIBMATTI_B3D_GlStateManager_Clear(LIBMATTI_GL_GL_COLOR_BUFFER_BIT);

    // Java: src Y are flipped, since our FB is flipped
    LIBMATTI_GL_glBlitFramebuffer(0, fb->height, fb->width, 0,
                                  clamp(wleft, 0, windowFBWidth),
                                  clamp(wtop, 0, windowFBHeight),
                                  clamp(wright, 0, windowFBWidth),
                                  clamp(wbottom, 0, windowFBHeight),
                                  LIBMATTI_GL_GL_COLOR_BUFFER_BIT, LIBMATTI_GL_GL_NEAREST);

    LIBMATTI_B3D_GlStateManager_BindFramebuffer(LIBMATTI_GL_GL_FRAMEBUFFER, 0);
}

// Java: public void close()
void LIBMATTI_FML_EarlyFramebuffer_Free(LIBMATTI_FML_EarlyFramebuffer *fb)
{
    if (fb == NULL)
        return;
    LIBMATTI_GL_glDeleteTextures(1, &fb->texture);
    LIBMATTI_B3D_GlStateManager_DeleteFramebuffers(fb->framebuffer);
    free(fb);
}

int LIBMATTI_FML_EarlyFramebuffer_Width(const LIBMATTI_FML_EarlyFramebuffer *fb)
{
    return fb->width;
}

int LIBMATTI_FML_EarlyFramebuffer_Height(const LIBMATTI_FML_EarlyFramebuffer *fb)
{
    return fb->height;
}

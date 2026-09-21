// Port of net.neoforged.fml.earlydisplay.render.EarlyFramebuffer.
// Java renders the theme layout into this offscreen framebuffer and blits it to
// the screen with a vertically flipped source rectangle ("src Y are flipped,
// since our FB is flipped"). That flip is what keeps the y-down screen-pixel
// coordinate system of the element shaders upright on the default framebuffer.

#ifndef MATTICRAFT_FML_EARLYDISPLAY_EARLYFRAMEBUFFER_H
#define MATTICRAFT_FML_EARLYDISPLAY_EARLYFRAMEBUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct LIBMATTI_FML_EarlyFramebuffer LIBMATTI_FML_EarlyFramebuffer;

// Java: new EarlyFramebuffer(width, height)
LIBMATTI_FML_EarlyFramebuffer *LIBMATTI_FML_EarlyFramebuffer_New(int width, int height);

// Java: public void resize(int, int) - reallocates the texture when the size changed
void LIBMATTI_FML_EarlyFramebuffer_Resize(LIBMATTI_FML_EarlyFramebuffer *framebuffer, int width, int height);

// Java: public void activate() / deactivate()
void LIBMATTI_FML_EarlyFramebuffer_Activate(LIBMATTI_FML_EarlyFramebuffer *framebuffer);
void LIBMATTI_FML_EarlyFramebuffer_Deactivate(LIBMATTI_FML_EarlyFramebuffer *framebuffer);

// Java: public void blitToScreen(ThemeColor backgroundColor, int windowFBWidth, int windowFBHeight)
// The colour components are floats in 0..1 (ThemeColor r/g/b).
void LIBMATTI_FML_EarlyFramebuffer_BlitToScreen(LIBMATTI_FML_EarlyFramebuffer *framebuffer, float backgroundR,
                                                float backgroundG, float backgroundB, int windowFBWidth,
                                                int windowFBHeight);

// Java: public void close()
void LIBMATTI_FML_EarlyFramebuffer_Free(LIBMATTI_FML_EarlyFramebuffer *framebuffer);

int LIBMATTI_FML_EarlyFramebuffer_Width(const LIBMATTI_FML_EarlyFramebuffer *framebuffer);
int LIBMATTI_FML_EarlyFramebuffer_Height(const LIBMATTI_FML_EarlyFramebuffer *framebuffer);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_FML_EARLYDISPLAY_EARLYFRAMEBUFFER_H

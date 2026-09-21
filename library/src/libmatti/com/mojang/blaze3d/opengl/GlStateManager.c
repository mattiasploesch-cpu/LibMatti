// Port of com.mojang.blaze3d.opengl.GlStateManager - the state cache layer.
// Every setter compares against the cached struct first and only touches GL on
// a real change, exactly like the Java original. Thread assertions go through
// RenderSystem (implemented in RenderSystem.c).

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"

#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <string.h>

// Java: private static final GlStateManager.BlendState BLEND = ...
// Java: private static final GlStateManager.BlendState BLEND = ...
static LIBMATTI_B3D_BlendState BLEND = {0, 1, 0, 1, 0};
// Java: private static final GlStateManager.DepthState DEPTH = ...
static LIBMATTI_B3D_DepthState DEPTH = {0, 1, 513};
// Java: private static final GlStateManager.CullState CULL = ...
static LIBMATTI_B3D_CullState CULL = {0};
// Java: private static final GlStateManager.PolygonOffsetState POLY_OFFSET = ...
static LIBMATTI_B3D_PolygonOffsetState POLY_OFFSET = {0, 0.0f, 0.0f};
// Java: private static final GlStateManager.ColorLogicState COLOR_LOGIC = ...
static LIBMATTI_B3D_ColorLogicState COLOR_LOGIC = {0, 5379};
// Java: private static final GlStateManager.ScissorState SCISSOR = ...
static LIBMATTI_B3D_ScissorState SCISSOR = {0};
// Java: private static int activeTexture;
static int activeTexture = 0;
// Java: private static final int TEXTURE_COUNT = 12;
#define TEXTURE_COUNT 12
// Java: private static final GlStateManager.TextureState[] TEXTURES = ...
static LIBMATTI_B3D_TextureState TEXTURES[TEXTURE_COUNT];
// Java: private static final GlStateManager.ColorMask COLOR_MASK = ...
static LIBMATTI_B3D_ColorMask COLOR_MASK = {1, 1, 1, 1};
// Java: private static int readFbo; private static int writeFbo;
static int readFbo = 0;
static int writeFbo = 0;

const LIBMATTI_B3D_DepthState *LIBMATTI_B3D_GlStateManager_GetDepth(void)
{
    return &DEPTH;
}

const LIBMATTI_B3D_BlendState *LIBMATTI_B3D_GlStateManager_GetBlend(void)
{
    return &BLEND;
}

const LIBMATTI_B3D_ColorMask *LIBMATTI_B3D_GlStateManager_GetColorMask(void)
{
    return &COLOR_MASK;
}

int LIBMATTI_B3D_GlStateManager_GetActiveTexture(void)
{
    return activeTexture;
}

// Java: private static void _enable(int) / _disable(int) through BooleanState.
static void boolean_state_set(int *enabled, int state, int value)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (value != *enabled)
    {
        *enabled = value;
        if (value)
            LIBMATTI_GL_glEnable(state);
        else
            LIBMATTI_GL_glDisable(state);
    }
}

void LIBMATTI_B3D_GlStateManager_EnableDepthTest(void)
{
    boolean_state_set(&DEPTH.mode, LIBMATTI_GL_GL_DEPTH_TEST, 1);
}

void LIBMATTI_B3D_GlStateManager_DisableDepthTest(void)
{
    boolean_state_set(&DEPTH.mode, LIBMATTI_GL_GL_DEPTH_TEST, 0);
}

void LIBMATTI_B3D_GlStateManager_DepthFunc(int func)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (func != DEPTH.func)
    {
        DEPTH.func = func;
        LIBMATTI_GL_glDepthFunc(func);
    }
}

void LIBMATTI_B3D_GlStateManager_DepthMask(int mask)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (mask != DEPTH.mask)
    {
        DEPTH.mask = mask;
        LIBMATTI_GL_glDepthMask(mask);
    }
}

void LIBMATTI_B3D_GlStateManager_EnableBlend(void)
{
    boolean_state_set(&BLEND.mode, LIBMATTI_GL_GL_BLEND, 1);
}

void LIBMATTI_B3D_GlStateManager_DisableBlend(void)
{
    boolean_state_set(&BLEND.mode, LIBMATTI_GL_GL_BLEND, 0);
}

void LIBMATTI_B3D_GlStateManager_BlendFuncSeparate(int srcRgb, int dstRgb, int srcAlpha, int dstAlpha)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (srcRgb != BLEND.srcRgb || dstRgb != BLEND.dstRgb || srcAlpha != BLEND.srcAlpha || dstAlpha != BLEND.dstAlpha)
    {
        BLEND.srcRgb = srcRgb;
        BLEND.dstRgb = dstRgb;
        BLEND.srcAlpha = srcAlpha;
        BLEND.dstAlpha = dstAlpha;
        LIBMATTI_GL_glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
    }
}

void LIBMATTI_B3D_GlStateManager_EnableCull(void)
{
    boolean_state_set(&CULL.enable, LIBMATTI_GL_GL_CULL_FACE, 1);
}

void LIBMATTI_B3D_GlStateManager_DisableCull(void)
{
    boolean_state_set(&CULL.enable, LIBMATTI_GL_GL_CULL_FACE, 0);
}

void LIBMATTI_B3D_GlStateManager_EnablePolygonOffset(void)
{
    boolean_state_set(&POLY_OFFSET.fill, LIBMATTI_GL_GL_POLYGON_OFFSET_FILL, 1);
}

void LIBMATTI_B3D_GlStateManager_DisablePolygonOffset(void)
{
    boolean_state_set(&POLY_OFFSET.fill, LIBMATTI_GL_GL_POLYGON_OFFSET_FILL, 0);
}

void LIBMATTI_B3D_GlStateManager_PolygonOffset(float factor, float units)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (factor != POLY_OFFSET.factor || units != POLY_OFFSET.units)
    {
        POLY_OFFSET.factor = factor;
        POLY_OFFSET.units = units;
        LIBMATTI_GL_glPolygonOffset(factor, units);
    }
}

void LIBMATTI_B3D_GlStateManager_EnableColorLogicOp(void)
{
    boolean_state_set(&COLOR_LOGIC.enable, LIBMATTI_GL_GL_COLOR_LOGIC_OP, 1);
}

void LIBMATTI_B3D_GlStateManager_DisableColorLogicOp(void)
{
    boolean_state_set(&COLOR_LOGIC.enable, LIBMATTI_GL_GL_COLOR_LOGIC_OP, 0);
}

void LIBMATTI_B3D_GlStateManager_LogicOp(int op)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (op != COLOR_LOGIC.op)
    {
        COLOR_LOGIC.op = op;
        LIBMATTI_GL_glLogicOp((unsigned int) op);
    }
}

void LIBMATTI_B3D_GlStateManager_EnableScissorTest(void)
{
    boolean_state_set(&SCISSOR.mode, LIBMATTI_GL_GL_SCISSOR_TEST, 1);
}

void LIBMATTI_B3D_GlStateManager_DisableScissorTest(void)
{
    boolean_state_set(&SCISSOR.mode, LIBMATTI_GL_GL_SCISSOR_TEST, 0);
}

void LIBMATTI_B3D_GlStateManager_ScissorBox(int x, int y, int width, int height)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glScissor(x, y, width, height);
}

void LIBMATTI_B3D_GlStateManager_ActiveTexture(int texture)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (activeTexture != texture - LIBMATTI_GL_GL_TEXTURE0)
    {
        activeTexture = texture - LIBMATTI_GL_GL_TEXTURE0;
        LIBMATTI_GL_glActiveTexture(texture);
    }
}

void LIBMATTI_B3D_GlStateManager_BindTexture(int texture)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (texture != TEXTURES[activeTexture].binding)
    {
        TEXTURES[activeTexture].binding = texture;
        LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, texture);
    }
}

int LIBMATTI_B3D_GlStateManager_GenTexture(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    unsigned int texture = 0;
    LIBMATTI_GL_glGenTextures(1, &texture);
    return (int) texture;
}

void LIBMATTI_B3D_GlStateManager_DeleteTexture(int texture)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDeleteTextures(1, (const unsigned int *) &texture);
    // Java: clear the binding in every texture unit state.
    for (int i = 0; i < TEXTURE_COUNT; i++)
    {
        if (TEXTURES[i].binding == texture)
            TEXTURES[i].binding = -1;
    }
}

void LIBMATTI_B3D_GlStateManager_TexParameter(int target, int pname, int value)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glTexParameteri((unsigned int) target, (unsigned int) pname, value);
}

int LIBMATTI_B3D_GlStateManager_GetTexLevelParameter(int target, int level, int pname)
{
    return LIBMATTI_GL_glGetTexLevelParameteri((unsigned int) target, level, (unsigned int) pname);
}

void LIBMATTI_B3D_GlStateManager_TexImage2D(int target, int level, int internalFormat,
                                            int width, int height, int border,
                                            int format, int type, const void *pixels)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glTexImage2D((unsigned int) target, level, internalFormat, width, height, border,
                             (unsigned int) format, (unsigned int) type, pixels);
}

void LIBMATTI_B3D_GlStateManager_TexSubImage2D(int target, int level, int xOffset, int yOffset,
                                               int width, int height, int format, int type,
                                               const void *pixels)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glTexSubImage2D((unsigned int) target, level, xOffset, yOffset, width, height,
                                (unsigned int) format, (unsigned int) type, pixels);
}

void LIBMATTI_B3D_GlStateManager_TexSubImage2DOffset(int target, int level, int xOffset, int yOffset,
                                                     int width, int height, int format, int type,
                                                     long offset)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glTexSubImage2D((unsigned int) target, level, xOffset, yOffset, width, height,
                                (unsigned int) format, (unsigned int) type, (const void *) offset);
}

void LIBMATTI_B3D_GlStateManager_Viewport(int x, int y, int width, int height)
{
    LIBMATTI_GL_glViewport(x, y, width, height);
}

void LIBMATTI_B3D_GlStateManager_ColorMask(int red, int green, int blue, int alpha)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    if (red != COLOR_MASK.red || green != COLOR_MASK.green || blue != COLOR_MASK.blue || alpha != COLOR_MASK.alpha)
    {
        COLOR_MASK.red = red;
        COLOR_MASK.green = green;
        COLOR_MASK.blue = blue;
        COLOR_MASK.alpha = alpha;
        LIBMATTI_GL_glColorMask(red, green, blue, alpha);
    }
}

void LIBMATTI_B3D_GlStateManager_Clear(int mask)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glClear(mask);
}

// Java: GlStateManager._clearColor(float, float, float, float)
void LIBMATTI_B3D_GlStateManager_ClearColor(float red, float green, float blue, float alpha)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glClearColor(red, green, blue, alpha);
}

void LIBMATTI_B3D_GlStateManager_PixelStore(int pname, int value)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glPixelStorei(pname, value);
}

void LIBMATTI_B3D_GlStateManager_ReadPixels(int x, int y, int width, int height,
                                            int format, int type, long offset)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glReadPixels(x, y, width, height, (unsigned int) format, (unsigned int) type, (void *) offset);
}

int LIBMATTI_B3D_GlStateManager_GetError(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return (int) LIBMATTI_GL_glGetError();
}

void LIBMATTI_B3D_GlStateManager_ClearGlErrors(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    while (LIBMATTI_GL_glGetError() != 0)
    {
    }
}

const char *LIBMATTI_B3D_GlStateManager_GetString(int name)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glGetString(name);
}

int LIBMATTI_B3D_GlStateManager_GetInteger(int name)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glGetInteger(name);
}

unsigned int LIBMATTI_B3D_GlStateManager_GenBuffers(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    unsigned int buffer = 0;
    LIBMATTI_GL_glGenBuffers(1, &buffer);
    return buffer;
}

void LIBMATTI_B3D_GlStateManager_DeleteBuffers(unsigned int buffer)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDeleteBuffers(1, &buffer);
}

void LIBMATTI_B3D_GlStateManager_BindBuffer(unsigned int target, unsigned int buffer)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glBindBuffer(target, buffer);
}

void LIBMATTI_B3D_GlStateManager_BindVertexArray(unsigned int array)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glBindVertexArray(array);
}

unsigned int LIBMATTI_B3D_GlStateManager_GenVertexArrays(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    unsigned int array = 0;
    LIBMATTI_GL_glGenVertexArrays(1, &array);
    return array;
}

void LIBMATTI_B3D_GlStateManager_DeleteVertexArrays(int n, const unsigned int *arrays)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDeleteVertexArrays(n, arrays);
}

void LIBMATTI_B3D_GlStateManager_BufferData(unsigned int target, const void *data, long size, unsigned int usage)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glBufferData(target, size, data, usage);
}

void LIBMATTI_B3D_GlStateManager_BufferDataSized(unsigned int target, long size, unsigned int usage)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glBufferData(target, size, NULL, usage);
}

void LIBMATTI_B3D_GlStateManager_BufferSubData(unsigned int target, long offset, const void *data, long size)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glBufferSubData(target, offset, size, data);
}

void *LIBMATTI_B3D_GlStateManager_MapBufferRange(unsigned int target, long offset, long size, unsigned int access)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glMapBufferRange(target, offset, size, access);
}

int LIBMATTI_B3D_GlStateManager_UnmapBuffer(unsigned int target)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glUnmapBuffer(target);
}

void LIBMATTI_B3D_GlStateManager_VertexAttribPointer(unsigned int index, int size, unsigned int type,
                                                     int normalized, int stride, const void *pointer)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void LIBMATTI_B3D_GlStateManager_VertexAttribIPointer(unsigned int index, int size, unsigned int type,
                                                      int stride, const void *pointer)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glVertexAttribIPointer(index, size, type, stride, pointer);
}

void LIBMATTI_B3D_GlStateManager_EnableVertexAttribArray(unsigned int index)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glEnableVertexAttribArray(index);
}

void LIBMATTI_B3D_GlStateManager_DrawElements(unsigned int mode, int count, unsigned int type, const void *indices)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDrawElements(mode, count, type, indices);
}

void LIBMATTI_B3D_GlStateManager_DrawArrays(unsigned int mode, int first, int count)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDrawArrays(mode, first, count);
}

int LIBMATTI_B3D_GlStateManager_CreateShader(unsigned int type)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glCreateShader(type);
}

void LIBMATTI_B3D_GlStateManager_ShaderSource(int shader, const char *source)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glShaderSource((unsigned int) shader, source);
}

void LIBMATTI_B3D_GlStateManager_CompileShader(int shader)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glCompileShader(shader);
}

int LIBMATTI_B3D_GlStateManager_GetShaderi(int shader, int pname)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glGetShaderi(shader, pname);
}

void LIBMATTI_B3D_GlStateManager_AttachShader(int program, int shader)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glAttachShader(program, shader);
}

void LIBMATTI_B3D_GlStateManager_DeleteShader(int shader)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDeleteShader(shader);
}

int LIBMATTI_B3D_GlStateManager_CreateProgram(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glCreateProgram();
}

void LIBMATTI_B3D_GlStateManager_LinkProgram(int program)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glLinkProgram(program);
}

int LIBMATTI_B3D_GlStateManager_GetProgrami(int program, int pname)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glGetProgrami(program, pname);
}

void LIBMATTI_B3D_GlStateManager_DeleteProgram(int program)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDeleteProgram(program);
}

void LIBMATTI_B3D_GlStateManager_UseProgram(int program)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glUseProgram(program);
}

int LIBMATTI_B3D_GlStateManager_GetUniformLocation(int program, const char *name)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glGetUniformLocation(program, name);
}

void LIBMATTI_B3D_GlStateManager_Uniform1i(int location, int value)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glUniform1i(location, value);
}

void LIBMATTI_B3D_GlStateManager_BindAttribLocation(int program, unsigned int index, const char *name)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glBindAttribLocation(program, index, name);
}

void LIBMATTI_B3D_GlStateManager_GetShaderInfoLog(int shader, int maxLength, int *length, char *infoLog)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    (void) maxLength;
    // Java: GL20.glGetShaderInfoLog returns the log string; the binding's
    // variant returns a driver-owned buffer (never NULL after link failure).
    const char *log = LIBMATTI_GL_glGetShaderInfoLog((unsigned int) shader);
    if (infoLog != NULL)
    {
        size_t len = log != NULL ? strlen(log) : 0;
        if (length != NULL)
            *length = (int) len;
        if (len > 0)
            memcpy(infoLog, log, len < (size_t) maxLength ? len + 1 : (size_t) maxLength);
    }
}

void LIBMATTI_B3D_GlStateManager_GetProgramInfoLog(int program, int maxLength, int *length, char *infoLog)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    (void) maxLength;
    const char *log = LIBMATTI_GL_glGetProgramInfoLog((unsigned int) program);
    if (infoLog != NULL)
    {
        size_t len = log != NULL ? strlen(log) : 0;
        if (length != NULL)
            *length = (int) len;
        if (len > 0)
            memcpy(infoLog, log, len < (size_t) maxLength ? len + 1 : (size_t) maxLength);
    }
}

void LIBMATTI_B3D_GlStateManager_BindFramebuffer(unsigned int target, unsigned int framebuffer)
{
    // Java: READ (36008) and WRITE (36009) are tracked independently; binding
    // GL_FRAMEBUFFER (36160) updates both.
    if ((target == 36008 || target == 36160) && readFbo != (int) framebuffer)
    {
        LIBMATTI_GL_glBindFramebuffer(36008, framebuffer);
        readFbo = (int) framebuffer;
    }

    if ((target == 36009 || target == 36160) && writeFbo != (int) framebuffer)
    {
        LIBMATTI_GL_glBindFramebuffer(36009, framebuffer);
        writeFbo = (int) framebuffer;
    }
}

int LIBMATTI_B3D_GlStateManager_GetFrameBuffer(unsigned int target)
{
    if (target == 36008)
        return readFbo;
    return target == 36009 ? writeFbo : 0;
}

int LIBMATTI_B3D_GlStateManager_GenFramebuffers(void)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    unsigned int framebuffer = 0;
    LIBMATTI_GL_glGenFramebuffers(1, &framebuffer);
    return (int) framebuffer;
}

void LIBMATTI_B3D_GlStateManager_DeleteFramebuffers(unsigned int framebuffer)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDeleteFramebuffers(1, &framebuffer);
    if (readFbo == (int) framebuffer)
        readFbo = 0;

    if (writeFbo == (int) framebuffer)
        writeFbo = 0;
}

void LIBMATTI_B3D_GlStateManager_FramebufferTexture2D(unsigned int target, unsigned int attachment,
                                                      unsigned int textureTarget, int texture, int level)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glFramebufferTexture2D(target, attachment, textureTarget, texture, level);
}

void LIBMATTI_B3D_GlStateManager_BlitFramebuffer(int srcX0, int srcY0, int srcX1, int srcY1,
                                                 int dstX0, int dstY0, int dstX1, int dstY1,
                                                 int mask, int filter)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void *LIBMATTI_B3D_GlStateManager_FenceSync(unsigned int condition, unsigned int flags)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glFenceSync(condition, flags);
}

int LIBMATTI_B3D_GlStateManager_ClientWaitSync(void *sync, unsigned int flags, unsigned long long timeout)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    return LIBMATTI_GL_glClientWaitSync(sync, flags, timeout);
}
void LIBMATTI_B3D_GlStateManager_DeleteSync(void *sync)
{
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    LIBMATTI_GL_glDeleteSync(sync);
}

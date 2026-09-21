// Port of com.mojang.blaze3d.opengl.GlStateManager (1.21.11: the GL state
// cache layer). Java caches every GL state in static structs so repeated
// setters with the same value are no-ops. The C port keeps the same state
// structs; every call routes through LIBMATTI_GL_* and asserts the render
// thread like RenderSystem.assertOnRenderThread().

#ifndef MATTICRAFT_BLAZE3D_OPENGL_GLSTATESMANAGER_H
#define MATTICRAFT_BLAZE3D_OPENGL_GLSTATESMANAGER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: GlStateManager.TextureState - binding per texture unit.
typedef struct LIBMATTI_B3D_TextureState
{
    int binding;
} LIBMATTI_B3D_TextureState;

// Java: GlStateManager.DepthState - mode is a BooleanState(GL_DEPTH_TEST).
typedef struct LIBMATTI_B3D_DepthState
{
    int mode;   // 1 = enabled, 0 = disabled (BooleanState.enabled)
    int mask;   // 1 = true (GL depth mask)
    int func;   // GL enum, default GL_LESS (513)
} LIBMATTI_B3D_DepthState;

// Java: GlStateManager.BlendState - mode is a BooleanState(GL_BLEND).
typedef struct LIBMATTI_B3D_BlendState
{
    int mode;      // 1 = enabled
    int srcRgb;    // default GL_ONE (1)
    int dstRgb;    // default GL_ZERO (0)
    int srcAlpha;  // default GL_ONE (1)
    int dstAlpha;  // default GL_ZERO (0)
} LIBMATTI_B3D_BlendState;

// Java: GlStateManager.CullState - enable is a BooleanState(GL_CULL_FACE).
typedef struct LIBMATTI_B3D_CullState
{
    int enable;
} LIBMATTI_B3D_CullState;

// Java: GlStateManager.PolygonOffsetState - fill is BooleanState(GL_POLYGON_OFFSET_FILL).
typedef struct LIBMATTI_B3D_PolygonOffsetState
{
    int fill;
    float factor;
    float units;
} LIBMATTI_B3D_PolygonOffsetState;

// Java: GlStateManager.ColorLogicState - enable is BooleanState(GL_COLOR_LOGIC_OP).
typedef struct LIBMATTI_B3D_ColorLogicState
{
    int enable;
    int op;  // default GL_COPY (5379)
} LIBMATTI_B3D_ColorLogicState;

// Java: GlStateManager.ScissorState - mode is BooleanState(GL_SCISSOR_TEST).
typedef struct LIBMATTI_B3D_ScissorState
{
    int mode;
} LIBMATTI_B3D_ScissorState;

// Java: GlStateManager.ColorMask.
typedef struct LIBMATTI_B3D_ColorMask
{
    int red;
    int green;
    int blue;
    int alpha;
} LIBMATTI_B3D_ColorMask;

// Java: the static GlStateManager state. Java holds these as static final
// fields; the port hands out one const instance the renderer can inspect.
const LIBMATTI_B3D_DepthState *LIBMATTI_B3D_GlStateManager_GetDepth(void);
const LIBMATTI_B3D_BlendState *LIBMATTI_B3D_GlStateManager_GetBlend(void);
const LIBMATTI_B3D_ColorMask *LIBMATTI_B3D_GlStateManager_GetColorMask(void);
int LIBMATTI_B3D_GlStateManager_GetActiveTexture(void);

// ---- BooleanState-backed toggles -----------------------------------------
// Java: GlStateManager._enableDepthTest / _disableDepthTest
void LIBMATTI_B3D_GlStateManager_EnableDepthTest(void);
void LIBMATTI_B3D_GlStateManager_DisableDepthTest(void);
// Java: GlStateManager._depthFunc(int)
void LIBMATTI_B3D_GlStateManager_DepthFunc(int func);
// Java: GlStateManager._depthMask(boolean)
void LIBMATTI_B3D_GlStateManager_DepthMask(int mask);

// Java: GlStateManager._enableBlend / _disableBlend
void LIBMATTI_B3D_GlStateManager_EnableBlend(void);
void LIBMATTI_B3D_GlStateManager_DisableBlend(void);
// Java: GlStateManager._blendFuncSeparate(int, int, int, int)
void LIBMATTI_B3D_GlStateManager_BlendFuncSeparate(int srcRgb, int dstRgb, int srcAlpha, int dstAlpha);

// Java: GlStateManager._enableCull / _disableCull
void LIBMATTI_B3D_GlStateManager_EnableCull(void);
void LIBMATTI_B3D_GlStateManager_DisableCull(void);

// Java: GlStateManager._enablePolygonOffset / _disablePolygonOffset / _polygonOffset
void LIBMATTI_B3D_GlStateManager_EnablePolygonOffset(void);
void LIBMATTI_B3D_GlStateManager_DisablePolygonOffset(void);
void LIBMATTI_B3D_GlStateManager_PolygonOffset(float factor, float units);

// Java: GlStateManager._enableColorLogicOp / _disableColorLogicOp / _logicOp
void LIBMATTI_B3D_GlStateManager_EnableColorLogicOp(void);
void LIBMATTI_B3D_GlStateManager_DisableColorLogicOp(void);
void LIBMATTI_B3D_GlStateManager_LogicOp(int op);

// Java: GlStateManager._enableScissorTest / _disableScissorTest / _scissorBox
void LIBMATTI_B3D_GlStateManager_EnableScissorTest(void);
void LIBMATTI_B3D_GlStateManager_DisableScissorTest(void);
void LIBMATTI_B3D_GlStateManager_ScissorBox(int x, int y, int width, int height);

// ---- Textures -------------------------------------------------------------
// Java: GlStateManager._activeTexture(int) - takes the GL enum, stores unit-33984.
void LIBMATTI_B3D_GlStateManager_ActiveTexture(int texture);
// Java: GlStateManager._bindTexture(int) - binds GL_TEXTURE_2D on the active unit.
void LIBMATTI_B3D_GlStateManager_BindTexture(int texture);
// Java: GlStateManager._genTexture / _deleteTexture
int LIBMATTI_B3D_GlStateManager_GenTexture(void);
void LIBMATTI_B3D_GlStateManager_DeleteTexture(int texture);
// Java: GlStateManager._texParameter(int, int, int)
void LIBMATTI_B3D_GlStateManager_TexParameter(int target, int pname, int value);
// Java: GlStateManager._getTexLevelParameter(int, int, int)
int LIBMATTI_B3D_GlStateManager_GetTexLevelParameter(int target, int level, int pname);
// Java: GlStateManager._texImage2D(...) - pixels may be NULL.
void LIBMATTI_B3D_GlStateManager_TexImage2D(int target, int level, int internalFormat,
                                            int width, int height, int border,
                                            int format, int type, const void *pixels);
// Java: GlStateManager._texSubImage2D(..., long offset) - client pointer variant.
void LIBMATTI_B3D_GlStateManager_TexSubImage2D(int target, int level, int xOffset, int yOffset,
                                               int width, int height, int format, int type,
                                               const void *pixels);
// Java: GlStateManager._texSubImage2D(..., long offset) - PBO offset variant.
void LIBMATTI_B3D_GlStateManager_TexSubImage2DOffset(int target, int level, int xOffset, int yOffset,
                                                     int width, int height, int format, int type,
                                                     long offset);

// ---- Frame / clear --------------------------------------------------------
// Java: GlStateManager._viewport(int, int, int, int)
void LIBMATTI_B3D_GlStateManager_Viewport(int x, int y, int width, int height);
// Java: GlStateManager._colorMask(boolean, boolean, boolean, boolean)
void LIBMATTI_B3D_GlStateManager_ColorMask(int red, int green, int blue, int alpha);
// Java: GlStateManager._clear(int)
void LIBMATTI_B3D_GlStateManager_Clear(int mask);
// Java: GlStateManager._clearColor(float, float, float, float)
void LIBMATTI_B3D_GlStateManager_ClearColor(float red, float green, float blue, float alpha);
// Java: GlStateManager._pixelStore(int, int)
void LIBMATTI_B3D_GlStateManager_PixelStore(int pname, int value);
// Java: GlStateManager._readPixels(..., long offset)
void LIBMATTI_B3D_GlStateManager_ReadPixels(int x, int y, int width, int height,
                                            int format, int type, long offset);

// ---- Queries --------------------------------------------------------------
// Java: GlStateManager._getError / clearGlErrors / _getString / _getInteger
int LIBMATTI_B3D_GlStateManager_GetError(void);
void LIBMATTI_B3D_GlStateManager_ClearGlErrors(void);
const char *LIBMATTI_B3D_GlStateManager_GetString(int name);
int LIBMATTI_B3D_GlStateManager_GetInteger(int name);

// ---- Buffers --------------------------------------------------------------
// Java: GlStateManager._glGenBuffers / _glDeleteBuffers
unsigned int LIBMATTI_B3D_GlStateManager_GenBuffers(void);
void LIBMATTI_B3D_GlStateManager_DeleteBuffers(unsigned int buffer);
// Java: GlStateManager._glBindBuffer(int, int)
void LIBMATTI_B3D_GlStateManager_BindBuffer(unsigned int target, unsigned int buffer);
// Java: GlStateManager._glBindVertexArray(int)
void LIBMATTI_B3D_GlStateManager_BindVertexArray(unsigned int array);
// Java: GlStateManager._glGenVertexArrays
unsigned int LIBMATTI_B3D_GlStateManager_GenVertexArrays(void);
void LIBMATTI_B3D_GlStateManager_DeleteVertexArrays(int n, const unsigned int *arrays);
// Java: GlStateManager._glBufferData(int, ByteBuffer, int)
void LIBMATTI_B3D_GlStateManager_BufferData(unsigned int target, const void *data, long size, unsigned int usage);
// Java: GlStateManager._glBufferData(int, long, int) - the sized variant.
void LIBMATTI_B3D_GlStateManager_BufferDataSized(unsigned int target, long size, unsigned int usage);
// Java: GlStateManager._glBufferSubData(int, long, ByteBuffer)
void LIBMATTI_B3D_GlStateManager_BufferSubData(unsigned int target, long offset, const void *data, long size);
// Java: GlStateManager._glMapBufferRange(int, long, long, int)
void *LIBMATTI_B3D_GlStateManager_MapBufferRange(unsigned int target, long offset, long size, unsigned int access);
// Java: GlStateManager._glUnmapBuffer(int)
int LIBMATTI_B3D_GlStateManager_UnmapBuffer(unsigned int target);

// ---- Vertex input / draw --------------------------------------------------
// Java: GlStateManager._vertexAttribPointer(int, int, int, boolean, int, long)
void LIBMATTI_B3D_GlStateManager_VertexAttribPointer(unsigned int index, int size, unsigned int type,
                                                     int normalized, int stride, const void *pointer);
// Java: GlStateManager._vertexAttribIPointer(int, int, int, int, long)
void LIBMATTI_B3D_GlStateManager_VertexAttribIPointer(unsigned int index, int size, unsigned int type,
                                                      int stride, const void *pointer);
// Java: GlStateManager._enableVertexAttribArray(int)
void LIBMATTI_B3D_GlStateManager_EnableVertexAttribArray(unsigned int index);
// Java: GlStateManager._drawElements(int, int, int, long)
void LIBMATTI_B3D_GlStateManager_DrawElements(unsigned int mode, int count, unsigned int type, const void *indices);
// Java: GlStateManager._drawArrays(int, int, int)
void LIBMATTI_B3D_GlStateManager_DrawArrays(unsigned int mode, int first, int count);

// ---- Shaders --------------------------------------------------------------
// Java: GlStateManager.glCreateShader / glShaderSource / glCompileShader / glGetShaderi
int LIBMATTI_B3D_GlStateManager_CreateShader(unsigned int type);
void LIBMATTI_B3D_GlStateManager_ShaderSource(int shader, const char *source);
void LIBMATTI_B3D_GlStateManager_CompileShader(int shader);
int LIBMATTI_B3D_GlStateManager_GetShaderi(int shader, int pname);
void LIBMATTI_B3D_GlStateManager_AttachShader(int program, int shader);
void LIBMATTI_B3D_GlStateManager_DeleteShader(int shader);
// Java: GlStateManager.glCreateProgram / glLinkProgram / glGetProgrami / glDeleteProgram
int LIBMATTI_B3D_GlStateManager_CreateProgram(void);
void LIBMATTI_B3D_GlStateManager_LinkProgram(int program);
int LIBMATTI_B3D_GlStateManager_GetProgrami(int program, int pname);
void LIBMATTI_B3D_GlStateManager_DeleteProgram(int program);
// Java: GlStateManager._glUseProgram(int)
void LIBMATTI_B3D_GlStateManager_UseProgram(int program);
// Java: GlStateManager._glGetUniformLocation / _glUniform1i / _glBindAttribLocation
int LIBMATTI_B3D_GlStateManager_GetUniformLocation(int program, const char *name);
void LIBMATTI_B3D_GlStateManager_Uniform1i(int location, int value);
void LIBMATTI_B3D_GlStateManager_BindAttribLocation(int program, unsigned int index, const char *name);
// Java: GlStateManager.glGetShaderInfoLog / glGetProgramInfoLog
void LIBMATTI_B3D_GlStateManager_GetShaderInfoLog(int shader, int maxLength, int *length, char *infoLog);
void LIBMATTI_B3D_GlStateManager_GetProgramInfoLog(int program, int maxLength, int *length, char *infoLog);

// ---- Framebuffers ---------------------------------------------------------
// Java: GlStateManager._glBindFramebuffer(int, int) - caches READ (36008) and
// WRITE (36009) independently like Java's readFbo/writeFbo pair.
void LIBMATTI_B3D_GlStateManager_BindFramebuffer(unsigned int target, unsigned int framebuffer);
// Java: GlStateManager.getFrameBuffer(int)
int LIBMATTI_B3D_GlStateManager_GetFrameBuffer(unsigned int target);
// Java: GlStateManager.glGenFramebuffers / _glDeleteFramebuffers
int LIBMATTI_B3D_GlStateManager_GenFramebuffers(void);
void LIBMATTI_B3D_GlStateManager_DeleteFramebuffers(unsigned int framebuffer);
// Java: GlStateManager._glFramebufferTexture2D(int, int, int, int, int)
void LIBMATTI_B3D_GlStateManager_FramebufferTexture2D(unsigned int target, unsigned int attachment,
                                                      unsigned int textureTarget, int texture, int level);
// Java: GlStateManager._glBlitFrameBuffer(...)
void LIBMATTI_B3D_GlStateManager_BlitFramebuffer(int srcX0, int srcY0, int srcX1, int srcY1,
                                                 int dstX0, int dstY0, int dstX1, int dstY1,
                                                 int mask, int filter);

// ---- Sync -----------------------------------------------------------------
// Java: GlStateManager._glFenceSync / _glClientWaitSync / _glDeleteSync
void *LIBMATTI_B3D_GlStateManager_FenceSync(unsigned int condition, unsigned int flags);
int LIBMATTI_B3D_GlStateManager_ClientWaitSync(void *sync, unsigned int flags, unsigned long long timeout);
void LIBMATTI_B3D_GlStateManager_DeleteSync(void *sync);

#ifdef __cplusplus
}
#endif

#endif

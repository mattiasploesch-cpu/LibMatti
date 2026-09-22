// Port of org.lwjgl.opengl (the GL binding: GL11, GL13, GL14, GL15, GL20,
// GL20C, GL30, GL31, GL32, GL32C, GL33C, ARBBufferStorage, ARBTimerQuery,
// ARBDirectStateAccess, ARBVertexAttribBinding, KHRDebug, EXTDebugLabel,
// ARBDebugOutput). The port resolves the native libGL at runtime with
// dlopen plus a glXGetProcAddress/eglGetProcAddress fallback chain, exactly
// like the GLFW binding dlopen()s libglfw. A missing or unloaded library
// means "not available" instead of a hard link error.
//
// Functions that are part of the ABI of every GL 1.1 driver (glBegin-era
// fixed function and the glGet* family) are additionally looked up with
// dlsym, like LWJGL's GLCapabilities.create() does for the core entry
// points.

#ifndef MATTICRAFT_LWJGL_OPENGL_GL_H
#define MATTICRAFT_LWJGL_OPENGL_GL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public interface GLDebugMessageCallbackI - (source, type, id, severity,
// length, message, userParam). The port passes the raw C strings; the length
// argument stays available like Java's.
typedef void (*LIBMATTI_GL_DebugMessageCallbackI)(unsigned int source, unsigned int type, unsigned int id,
                                                  unsigned int severity, int length, const char *message,
                                                  const void *userParam);

// Java: GL11.glGetError() returns 0 (GL_NO_ERROR) when no error is queued.
#define LIBMATTI_GL_GL_NO_ERROR 0

// The GL functions the game code calls, with the signatures of the C API.
// Java: GL11
void LIBMATTI_GL_glViewport(int x, int y, int width, int height);
void LIBMATTI_GL_glDepthFunc(unsigned int func);
void LIBMATTI_GL_glDepthMask(unsigned char flag);
void LIBMATTI_GL_glColorMask(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void LIBMATTI_GL_glStencilFunc(unsigned int func, int ref, unsigned int mask);
void LIBMATTI_GL_glStencilMask(unsigned int mask);
void LIBMATTI_GL_glStencilOp(unsigned int sfail, unsigned int dpfail, unsigned int dppass);
void LIBMATTI_GL_glClearStencil(int s);
void LIBMATTI_GL_glClearColor(float red, float green, float blue, float alpha);
void LIBMATTI_GL_glClear(unsigned int mask);
void LIBMATTI_GL_glLineWidth(float width);
void LIBMATTI_GL_glPolygonMode(unsigned int face, unsigned int mode);
void LIBMATTI_GL_glPolygonOffset(float factor, float units);
void LIBMATTI_GL_glScissor(int x, int y, int width, int height);
unsigned int LIBMATTI_GL_glGetError(void);
const char *LIBMATTI_GL_glGetString(unsigned int name);
int LIBMATTI_GL_glGetInteger(unsigned int pname);
float LIBMATTI_GL_glGetFloat(unsigned int pname);
void LIBMATTI_GL_glGetFloatv(unsigned int pname, float *params);
void LIBMATTI_GL_glReadPixels(int x, int y, int width, int height, unsigned int format, unsigned int type, void *pixels);
void LIBMATTI_GL_glEnable(unsigned int cap);
void LIBMATTI_GL_glDisable(unsigned int cap);
void LIBMATTI_GL_glLogicOp(unsigned int opcode);
void LIBMATTI_GL_glDrawBuffer(unsigned int mode);
// Java: GL13
void LIBMATTI_GL_glActiveTexture(unsigned int texture);
// Java: GL14
void LIBMATTI_GL_glBlendFuncSeparate(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha);
void LIBMATTI_GL_glBlendEquationSeparate(unsigned int modeRGB, unsigned int modeAlpha);
// Java: GL15
void LIBMATTI_GL_glGenBuffers(int n, unsigned int *buffers);
void LIBMATTI_GL_glDeleteBuffers(int n, const unsigned int *buffers);
void LIBMATTI_GL_glBindBuffer(unsigned int target, unsigned int buffer);
void LIBMATTI_GL_glBufferData(unsigned int target, long size, const void *data, unsigned int usage);
void LIBMATTI_GL_glBufferSubData(unsigned int target, long offset, long size, const void *data);
void *LIBMATTI_GL_glMapBufferRange(unsigned int target, long offset, long length, unsigned int access);
unsigned char LIBMATTI_GL_glUnmapBuffer(unsigned int target);
void LIBMATTI_GL_glFlushMappedBufferRange(unsigned int target, long offset, long length);
// Java: GL20 / GL20C
unsigned int LIBMATTI_GL_glCreateShader(unsigned int type);
void LIBMATTI_GL_glShaderSource(unsigned int shader, const char *source);
void LIBMATTI_GL_glCompileShader(unsigned int shader);
void LIBMATTI_GL_glDeleteShader(unsigned int shader);
int LIBMATTI_GL_glGetShaderi(unsigned int shader, unsigned int pname);
const char *LIBMATTI_GL_glGetShaderInfoLog(unsigned int shader);
// Java: GL20.glGetShaderiv(int, int, int*) - the status query (pname e.g. GL_COMPILE_STATUS)
int LIBMATTI_GL_glGetShaderiv(unsigned int shader, unsigned int pname);
// Java: GL20.glGetProgramiv(int, int, int*) - the status query (pname e.g. GL_LINK_STATUS)
int LIBMATTI_GL_glGetProgramiv(unsigned int program, unsigned int pname);
unsigned int LIBMATTI_GL_glCreateProgram(void);
void LIBMATTI_GL_glDeleteProgram(unsigned int program);
void LIBMATTI_GL_glAttachShader(unsigned int program, unsigned int shader);
void LIBMATTI_GL_glLinkProgram(unsigned int program);
int LIBMATTI_GL_glGetProgrami(unsigned int program, unsigned int pname);
const char *LIBMATTI_GL_glGetProgramInfoLog(unsigned int program);
void LIBMATTI_GL_glUseProgram(unsigned int program);
int LIBMATTI_GL_glGetUniformLocation(unsigned int program, const char *name);
void LIBMATTI_GL_glUniform1i(int location, int v0);
// Java: GL20.glUniform2f(int, float, float) / GL20.glUniform4f(int, float, float, float, float)
void LIBMATTI_GL_glUniform2f(int location, float v0, float v1);
void LIBMATTI_GL_glUniform4f(int location, float v0, float v1, float v2, float v3);
// Java: GL20.glUniform3f / GL20.glUniformMatrix4fv (the terrain uniforms).
void LIBMATTI_GL_glUniform3f(int location, float v0, float v1, float v2);
void LIBMATTI_GL_glUniformMatrix4fv(int location, int transpose, const float *value);
void LIBMATTI_GL_glBindAttribLocation(unsigned int program, unsigned int index, const char *name);
void LIBMATTI_GL_glEnableVertexAttribArray(unsigned int index);
void LIBMATTI_GL_glVertexAttribPointer(unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void *pointer);
void LIBMATTI_GL_glVertexAttribIPointer(unsigned int index, int size, unsigned int type, int stride, const void *pointer);
// Java: GL30
void LIBMATTI_GL_glGenVertexArrays(int n, unsigned int *arrays);
void LIBMATTI_GL_glDeleteVertexArrays(int n, const unsigned int *arrays);
void LIBMATTI_GL_glBindVertexArray(unsigned int array);
void LIBMATTI_GL_glGenFramebuffers(int n, unsigned int *framebuffers);
void LIBMATTI_GL_glDeleteFramebuffers(int n, const unsigned int *framebuffers);
void LIBMATTI_GL_glBindFramebuffer(unsigned int target, unsigned int framebuffer);
void LIBMATTI_GL_glFramebufferTexture2D(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level);
// Java: GL30.glGenRenderbuffers / glBindRenderbuffer / glRenderbufferStorage /
// glFramebufferRenderbuffer (the depth attachment the FBO needs).
void LIBMATTI_GL_glGenRenderbuffers(int n, unsigned int *renderbuffers);
void LIBMATTI_GL_glBindRenderbuffer(unsigned int target, unsigned int renderbuffer);
void LIBMATTI_GL_glRenderbufferStorage(unsigned int target, unsigned int internalformat, int width, int height);
void LIBMATTI_GL_glFramebufferRenderbuffer(unsigned int target, unsigned int attachment,
                                           unsigned int renderbuffertarget, unsigned int renderbuffer);
unsigned int LIBMATTI_GL_glCheckFramebufferStatus(unsigned int target);
void LIBMATTI_GL_glBlitFramebuffer(int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter);
void LIBMATTI_GL_glGenTextures(int n, unsigned int *textures);
void LIBMATTI_GL_glDeleteTextures(int n, const unsigned int *textures);
void LIBMATTI_GL_glBindTexture(unsigned int target, unsigned int texture);
void LIBMATTI_GL_glTexImage2D(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels);
void LIBMATTI_GL_glTexSubImage2D(unsigned int target, int level, int xoffset, int yoffset, int width, int height, unsigned int format, unsigned int type, const void *pixels);
void LIBMATTI_GL_glTexParameteri(unsigned int target, unsigned int pname, int param);
void LIBMATTI_GL_glTexParameterf(unsigned int target, unsigned int pname, float param);
int LIBMATTI_GL_glGetTexLevelParameteri(unsigned int target, int level, unsigned int pname);
// Java: GL31
void LIBMATTI_GL_glCopyBufferSubData(unsigned int readTarget, unsigned int writeTarget, long readOffset, long writeOffset, long size);
void LIBMATTI_GL_glTexBuffer(unsigned int target, unsigned int internalformat, unsigned int buffer);
void LIBMATTI_GL_glDrawArraysInstanced(unsigned int mode, int first, int count, int instancecount);
// Java: GL11.glDrawArrays(int mode, int first, int count)
void LIBMATTI_GL_glDrawArrays(unsigned int mode, int first, int count);
// Java: GL11.glDrawElements(int mode, int count, int type, long indices)
void LIBMATTI_GL_glDrawElements(unsigned int mode, int count, unsigned int type, const void *indices);
void LIBMATTI_GL_glDrawElementsInstanced(unsigned int mode, int count, unsigned int type, const void *indices, int instancecount);
int LIBMATTI_GL_glGetUniformBlockIndex(unsigned int program, const char *uniformBlockName);
void LIBMATTI_GL_glGetActiveUniformBlockName(unsigned int program, unsigned int uniformBlockIndex, char *uniformBlockName);
void LIBMATTI_GL_glUniformBlockBinding(unsigned int program, unsigned int uniformBlockIndex, unsigned int uniformBlockBinding);
// Java: GL32 / GL32C
void LIBMATTI_GL_glBindBufferRange(unsigned int target, unsigned int index, unsigned int buffer, long offset, long size);
void *LIBMATTI_GL_glFenceSync(unsigned int condition, unsigned int flags);
unsigned int LIBMATTI_GL_glClientWaitSync(void *sync, unsigned int flags, unsigned long long timeout);
void LIBMATTI_GL_glDeleteSync(void *sync);
void LIBMATTI_GL_glDrawElementsBaseVertex(unsigned int mode, int count, unsigned int type, const void *indices, int basevertex);
void LIBMATTI_GL_glDrawElementsInstancedBaseVertex(unsigned int mode, int count, unsigned int type, const void *indices, int instancecount, int basevertex);
// Java: GL33C
void LIBMATTI_GL_glGenSamplers(int n, unsigned int *samplers);
void LIBMATTI_GL_glDeleteSamplers(int n, const unsigned int *samplers);
void LIBMATTI_GL_glBindSampler(unsigned int unit, unsigned int sampler);
void LIBMATTI_GL_glSamplerParameteri(unsigned int sampler, unsigned int pname, int param);
void LIBMATTI_GL_glSamplerParameterf(unsigned int sampler, unsigned int pname, float param);
// Java: GL32C timer queries
void LIBMATTI_GL_glGenQueries(int n, unsigned int *ids);
void LIBMATTI_GL_glDeleteQueries(int n, const unsigned int *ids);
void LIBMATTI_GL_glBeginQuery(unsigned int target, unsigned int id);
void LIBMATTI_GL_glEndQuery(unsigned int target);
int LIBMATTI_GL_glGetQueryObjecti(unsigned int id, unsigned int pname);
long long LIBMATTI_GL_glGetQueryObjecti64(unsigned int id, unsigned int pname);
// Java: ARBBufferStorage
void LIBMATTI_GL_glBufferStorage(unsigned int target, long size, const void *data, unsigned int flags);
// Java: ARBDirectStateAccess (the Core implementation; the Emulated one maps to
// the bound-object calls like DirectStateAccess.Emulated)
unsigned int LIBMATTI_GL_glCreateBuffers(void);
void LIBMATTI_GL_glNamedBufferData(unsigned int buffer, long size, const void *data, unsigned int usage);
void LIBMATTI_GL_glNamedBufferStorage(unsigned int buffer, long size, const void *data, unsigned int flags);
void LIBMATTI_GL_glNamedBufferSubData(unsigned int buffer, long offset, long size, const void *data);
void *LIBMATTI_GL_glMapNamedBufferRange(unsigned int buffer, long offset, long length, unsigned int access);
void LIBMATTI_GL_glFlushMappedNamedBufferRange(unsigned int buffer, long offset, long length);
unsigned char LIBMATTI_GL_glUnmapNamedBuffer(unsigned int buffer);
void LIBMATTI_GL_glCopyNamedBufferSubData(unsigned int readBuffer, unsigned int writeBuffer, long readOffset, long writeOffset, long size);
unsigned int LIBMATTI_GL_glCreateFramebuffers(void);
void LIBMATTI_GL_glNamedFramebufferTexture(unsigned int framebuffer, unsigned int attachment, unsigned int texture, int level);
void LIBMATTI_GL_glBlitNamedFramebuffer(unsigned int readFramebuffer, unsigned int drawFramebuffer, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter);
// Java: ARBVertexAttribBinding (VertexArrayCache)
void LIBMATTI_GL_glVertexAttribFormat(unsigned int attribindex, int size, unsigned int type, unsigned char normalized, unsigned int relativeoffset);
void LIBMATTI_GL_glVertexAttribIFormat(unsigned int attribindex, int size, unsigned int type, unsigned int relativeoffset);
void LIBMATTI_GL_glVertexAttribBinding(unsigned int attribindex, unsigned int bindingindex);
void LIBMATTI_GL_glBindVertexBuffer(unsigned int bindingindex, unsigned int buffer, long offset, int stride);
// Java: KHRDebug / EXTDebugLabel
void LIBMATTI_GL_glObjectLabel(unsigned int identifier, unsigned int name, const char *label);
void LIBMATTI_GL_glPushDebugGroup(unsigned int source, unsigned int id, const char *message);
void LIBMATTI_GL_glPopDebugGroup(void);
void LIBMATTI_GL_glDebugMessageControl(unsigned int source, unsigned int type, unsigned int severity, int count, const unsigned int *ids, unsigned char enabled);
void LIBMATTI_GL_glDebugMessageCallback(LIBMATTI_GL_DebugMessageCallbackI callback, const void *userParam);
void LIBMATTI_GL_glDebugMessageControlARB(unsigned int source, unsigned int type, unsigned int severity, int count, const unsigned int *ids, unsigned char enabled);
void LIBMATTI_GL_glDebugMessageCallbackARB(LIBMATTI_GL_DebugMessageCallbackI callback, const void *userParam);
void LIBMATTI_GL_glLabelObjectEXT(unsigned int type, unsigned int object, const char *label);

// Java: LWJGL resolves the GL entry points once per context. The port exposes
// the same lifecycle: LIBMATTI_GL_Load() (re)resolves every function and
// reports whether the driver library was found, LIBMATTI_GL_Unload() drops the
// handles. The game calls Load after making the context current.
int LIBMATTI_GL_Load(void);
void LIBMATTI_GL_Unload(void);
// Java: GLCapabilities - 1 when the driver library and its glXGetProcAddress/
// eglGetProcAddress entry point are available.
int LIBMATTI_GL_IsAvailable(void);
// Java: the "GL_VENDOR GL_RENDERER GL_VERSION" line GlDevice renders during
// startup. The port caches it from the game's glGetString calls; NULL before
// one was made.
const char *LIBMATTI_GL_Describe(void);

// Java: GL11.glPixelStorei(int pname, int param)
void LIBMATTI_GL_glPixelStorei(unsigned int pname, int param);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_LWJGL_OPENGL_GL_H

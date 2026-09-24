// Port of the org.lwjgl.opengl binding implementation. LWJGL loads the GL
// entry points once per context through GLCapabilities.create(): every
// function is fetched through the platform's GetProcAddress (glXGetProcAddress
// on X11, eglGetProcAddress on Wayland) with a dlsym fallback for the GL 1.1
// ABI symbols. The port keeps exactly that chain behind dlopen, so a missing
// or unloaded driver library means "not available" instead of a hard link
// error.

#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: the native libGL. The Linux GL library ships as libGL.so.1 (GLX) with
// libEGL.so.1 as the Wayland path.
static void *gl_lib = NULL;
static void *gl_proc_get(void *unused, const char *name);
static unsigned char (*glXGetProcAddressARB)(const unsigned char *) = NULL;
static void *(*eglGetProcAddress)(const char *) = NULL;

// One resolver per entry point, exactly like LWJGL's generated
// GLCapabilities.create() function table.
#define GL_PROC(field, name)                                                                     \
    do                                                                                           \
    {                                                                                            \
        if (p_##field == NULL) p_##field = (void *) gl_proc_get(NULL, name);                     \
    } while (0)

static void *p_glViewport = NULL;
static void *p_glDepthFunc = NULL;
static void *p_glDepthMask = NULL;
static void *p_glColorMask = NULL;
static void *p_glStencilFunc = NULL;
static void *p_glStencilMask = NULL;
static void *p_glStencilOp = NULL;
static void *p_glClearStencil = NULL;
static void *p_glClearColor = NULL;
static void *p_glClear = NULL;
static void *p_glLineWidth = NULL;
static void *p_glPolygonMode = NULL;
static void *p_glPolygonOffset = NULL;
static void *p_glScissor = NULL;
static void *p_glGetError = NULL;
static void *p_glDrawArrays = NULL;
static void *p_glDrawElements = NULL;
static void *p_glPixelStorei = NULL;
static void *p_glGetString = NULL;
static void *p_glGetIntegerv = NULL;
static void *p_glGetFloatv = NULL;
static void *p_glReadPixels = NULL;
static void *p_glEnable = NULL;
static void *p_glDisable = NULL;
static void *p_glLogicOp = NULL;
static void *p_glDrawBuffer = NULL;
static void *p_glActiveTexture = NULL;
static void *p_glGetTexParameteriv = NULL;
static void *p_glGenerateMipmap = NULL;
static void *p_glBlendFuncSeparate = NULL;
static void *p_glBlendEquationSeparate = NULL;
static void *p_glGenBuffers = NULL;
static void *p_glDeleteBuffers = NULL;
static void *p_glBindBuffer = NULL;
static void *p_glBufferData = NULL;
static void *p_glBufferSubData = NULL;
static void *p_glMapBufferRange = NULL;
static void *p_glUnmapBuffer = NULL;
static void *p_glFlushMappedBufferRange = NULL;
static void *p_glCreateShader = NULL;
static void *p_glShaderSource = NULL;
static void *p_glCompileShader = NULL;
static void *p_glDeleteShader = NULL;
static void *p_glGetShaderiv = NULL;
static void *p_glGetShaderInfoLog = NULL;
static void *p_glCreateProgram = NULL;
static void *p_glDeleteProgram = NULL;
static void *p_glAttachShader = NULL;
static void *p_glLinkProgram = NULL;
static void *p_glGetProgramiv = NULL;
static void *p_glGetProgramInfoLog = NULL;
static void *p_glUseProgram = NULL;
static void *p_glGetUniformLocation = NULL;
static void *p_glGetUniformfv = NULL;
static void *p_glUniform1i = NULL;
static void *p_glUniform2f = NULL;
static void *p_glUniform4f = NULL;
static void *p_glUniform3f = NULL;
static void *p_glUniformMatrix4fv = NULL;
static void *p_glBindAttribLocation = NULL;
static void *p_glEnableVertexAttribArray = NULL;
static void *p_glVertexAttribPointer = NULL;
static void *p_glVertexAttribIPointer = NULL;
static void *p_glGenVertexArrays = NULL;
static void *p_glDeleteVertexArrays = NULL;
static void *p_glBindVertexArray = NULL;
static void *p_glGenFramebuffers = NULL;
static void *p_glDeleteFramebuffers = NULL;
static void *p_glBindFramebuffer = NULL;
static void *p_glFramebufferTexture2D = NULL;
static void *p_glGenRenderbuffers = NULL;
static void *p_glBindRenderbuffer = NULL;
static void *p_glRenderbufferStorage = NULL;
static void *p_glFramebufferRenderbuffer = NULL;
static void *p_glCheckFramebufferStatus = NULL;
static void *p_glBlitFramebuffer = NULL;
static void *p_glGenTextures = NULL;
static void *p_glDeleteTextures = NULL;
static void *p_glBindTexture = NULL;
static void *p_glTexImage2D = NULL;
static void *p_glTexSubImage2D = NULL;
static void *p_glTexParameteri = NULL;
static void *p_glTexParameterf = NULL;
static void *p_glGetTexLevelParameteriv = NULL;
static void *p_glCopyBufferSubData = NULL;
static void *p_glTexBuffer = NULL;
static void *p_glDrawArraysInstanced = NULL;
static void *p_glDrawElementsInstanced = NULL;
static void *p_glGetUniformBlockIndex = NULL;
static void *p_glGetActiveUniformBlockName = NULL;
static void *p_glUniformBlockBinding = NULL;
static void *p_glBindBufferRange = NULL;
static void *p_glFenceSync = NULL;
static void *p_glClientWaitSync = NULL;
static void *p_glDeleteSync = NULL;
static void *p_glDrawElementsBaseVertex = NULL;
static void *p_glDrawElementsInstancedBaseVertex = NULL;
static void *p_glGenSamplers = NULL;
static void *p_glDeleteSamplers = NULL;
static void *p_glBindSampler = NULL;
static void *p_glSamplerParameteri = NULL;
static void *p_glSamplerParameterf = NULL;
static void *p_glGenQueries = NULL;
static void *p_glDeleteQueries = NULL;
static void *p_glBeginQuery = NULL;
static void *p_glEndQuery = NULL;
static void *p_glGetQueryObjectiv = NULL;
static void *p_glGetQueryObjecti64v = NULL;
static void *p_glBufferStorage = NULL;
static void *p_glCreateBuffers = NULL;
static void *p_glNamedBufferData = NULL;
static void *p_glNamedBufferStorage = NULL;
static void *p_glNamedBufferSubData = NULL;
static void *p_glMapNamedBufferRange = NULL;
static void *p_glFlushMappedNamedBufferRange = NULL;
static void *p_glUnmapNamedBuffer = NULL;
static void *p_glCopyNamedBufferSubData = NULL;
static void *p_glCreateFramebuffers = NULL;
static void *p_glNamedFramebufferTexture = NULL;
static void *p_glBlitNamedFramebuffer = NULL;
static void *p_glVertexAttribFormat = NULL;
static void *p_glVertexAttribIFormat = NULL;
static void *p_glVertexAttribBinding = NULL;
static void *p_glBindVertexBuffer = NULL;
static void *p_glObjectLabel = NULL;
static void *p_glPushDebugGroup = NULL;
static void *p_glPopDebugGroup = NULL;
static void *p_glDebugMessageControl = NULL;
static void *p_glDebugMessageCallback = NULL;
static void *p_glDebugMessageControlARB = NULL;
static void *p_glDebugMessageCallbackARB = NULL;
static void *p_glLabelObjectEXT = NULL;

// Java: GLContext.getCapabilities() - the process-wide function table. The
// game resolves it once after the context is current; the port does the same.
static int gl_loaded = 0;
static int gl_have_description = 0;
static char gl_description[192];

// The ABI entry points every GL 1.1 driver exports; LWJGL falls back to the
// system loader for exactly these through NativeLibrary.getFunction.
static void *gl_proc_get(void *unused, const char *name)
{
    (void) unused;
    void *address = NULL;
    if (glXGetProcAddressARB != NULL)
        address = (void *) glXGetProcAddressARB((const unsigned char *) name);
    if (address == NULL && eglGetProcAddress != NULL)
        address = eglGetProcAddress(name);
    // Some drivers (Mesa's glXGetProcAddressARB without a bound context) hand
    // out tiny dispatch-table offsets instead of real entry points; anything
    // below one page is not executable code, so the lookup falls through to
    // the libGL ABI symbols (dlsym) instead of taking the bogus pointer.
    if ((uintptr_t) address < 4096)
        address = dlsym(gl_lib, name);
    if ((uintptr_t) address < 4096)
        return NULL;
    return address;
}

int LIBMATTI_GL_Load(void)
{
    if (gl_lib == NULL)
    {
        const char *names[] = {"libGL.so.1", "libGL.so", "libEGL.so.1", "libOSMesa.so.8", NULL};
        for (int i = 0; names[i] != NULL; i++)
        {
            gl_lib = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
            if (gl_lib != NULL) break;
        }
        if (gl_lib == NULL) return 0;
        // Java: GLCapabilities.create() resolves the GetProcAddress entry
        // points first; the per-function lookups use whichever answers.
        glXGetProcAddressARB = (unsigned char (*)(const unsigned char *)) dlsym(gl_lib, "glXGetProcAddressARB");
        eglGetProcAddress = (void *(*)(const char *)) dlsym(gl_lib, "eglGetProcAddress");
    }

    GL_PROC(glViewport, "glViewport");
    GL_PROC(glDepthFunc, "glDepthFunc");
    GL_PROC(glDepthMask, "glDepthMask");
    GL_PROC(glColorMask, "glColorMask");
    GL_PROC(glStencilFunc, "glStencilFunc");
    GL_PROC(glStencilMask, "glStencilMask");
    GL_PROC(glStencilOp, "glStencilOp");
    GL_PROC(glClearStencil, "glClearStencil");
    GL_PROC(glClearColor, "glClearColor");
    GL_PROC(glClear, "glClear");
    GL_PROC(glLineWidth, "glLineWidth");
    GL_PROC(glPolygonMode, "glPolygonMode");
    GL_PROC(glPolygonOffset, "glPolygonOffset");
    GL_PROC(glScissor, "glScissor");
    GL_PROC(glGetError, "glGetError");
    GL_PROC(glDrawArrays, "glDrawArrays");
    GL_PROC(glDrawElements, "glDrawElements");
    GL_PROC(glPixelStorei, "glPixelStorei");
    GL_PROC(glGetString, "glGetString");
    GL_PROC(glGetIntegerv, "glGetIntegerv");
    GL_PROC(glGetFloatv, "glGetFloatv");
    GL_PROC(glReadPixels, "glReadPixels");
    GL_PROC(glEnable, "glEnable");
    GL_PROC(glDisable, "glDisable");
    GL_PROC(glLogicOp, "glLogicOp");
    GL_PROC(glDrawBuffer, "glDrawBuffer");
    GL_PROC(glActiveTexture, "glActiveTexture");
    GL_PROC(glGetTexParameteriv, "glGetTexParameteriv");
    GL_PROC(glGenerateMipmap, "glGenerateMipmap");
    GL_PROC(glBlendFuncSeparate, "glBlendFuncSeparate");
    GL_PROC(glBlendEquationSeparate, "glBlendEquationSeparate");
    GL_PROC(glGenBuffers, "glGenBuffers");
    GL_PROC(glDeleteBuffers, "glDeleteBuffers");
    GL_PROC(glBindBuffer, "glBindBuffer");
    GL_PROC(glBufferData, "glBufferData");
    GL_PROC(glBufferSubData, "glBufferSubData");
    GL_PROC(glMapBufferRange, "glMapBufferRange");
    GL_PROC(glUnmapBuffer, "glUnmapBuffer");
    GL_PROC(glFlushMappedBufferRange, "glFlushMappedBufferRange");
    GL_PROC(glCreateShader, "glCreateShader");
    GL_PROC(glShaderSource, "glShaderSource");
    GL_PROC(glCompileShader, "glCompileShader");
    GL_PROC(glDeleteShader, "glDeleteShader");
    GL_PROC(glGetShaderiv, "glGetShaderiv");
    GL_PROC(glGetShaderInfoLog, "glGetShaderInfoLog");
    GL_PROC(glCreateProgram, "glCreateProgram");
    GL_PROC(glDeleteProgram, "glDeleteProgram");
    GL_PROC(glAttachShader, "glAttachShader");
    GL_PROC(glLinkProgram, "glLinkProgram");
    GL_PROC(glGetProgramiv, "glGetProgramiv");
    GL_PROC(glGetProgramInfoLog, "glGetProgramInfoLog");
    GL_PROC(glUseProgram, "glUseProgram");
    GL_PROC(glGetUniformLocation, "glGetUniformLocation");
    GL_PROC(glGetUniformfv, "glGetUniformfv");
    GL_PROC(glUniform1i, "glUniform1i");
    GL_PROC(glUniform2f, "glUniform2f");
    GL_PROC(glUniform4f, "glUniform4f");
    GL_PROC(glUniform3f, "glUniform3f");
    GL_PROC(glUniformMatrix4fv, "glUniformMatrix4fv");
    GL_PROC(glBindAttribLocation, "glBindAttribLocation");
    GL_PROC(glEnableVertexAttribArray, "glEnableVertexAttribArray");
    GL_PROC(glVertexAttribPointer, "glVertexAttribPointer");
    GL_PROC(glVertexAttribIPointer, "glVertexAttribIPointer");
    GL_PROC(glGenVertexArrays, "glGenVertexArrays");
    GL_PROC(glDeleteVertexArrays, "glDeleteVertexArrays");
    GL_PROC(glBindVertexArray, "glBindVertexArray");
    GL_PROC(glGenFramebuffers, "glGenFramebuffers");
    GL_PROC(glDeleteFramebuffers, "glDeleteFramebuffers");
    GL_PROC(glBindFramebuffer, "glBindFramebuffer");
    GL_PROC(glFramebufferTexture2D, "glFramebufferTexture2D");
    GL_PROC(glGenRenderbuffers, "glGenRenderbuffers");
    GL_PROC(glBindRenderbuffer, "glBindRenderbuffer");
    GL_PROC(glRenderbufferStorage, "glRenderbufferStorage");
    GL_PROC(glFramebufferRenderbuffer, "glFramebufferRenderbuffer");
    GL_PROC(glCheckFramebufferStatus, "glCheckFramebufferStatus");
    GL_PROC(glBlitFramebuffer, "glBlitFramebuffer");
    GL_PROC(glGenTextures, "glGenTextures");
    GL_PROC(glDeleteTextures, "glDeleteTextures");
    GL_PROC(glBindTexture, "glBindTexture");
    GL_PROC(glTexImage2D, "glTexImage2D");
    GL_PROC(glTexSubImage2D, "glTexSubImage2D");
    GL_PROC(glTexParameteri, "glTexParameteri");
    GL_PROC(glTexParameterf, "glTexParameterf");
    GL_PROC(glGetTexLevelParameteriv, "glGetTexLevelParameteriv");
    GL_PROC(glCopyBufferSubData, "glCopyBufferSubData");
    GL_PROC(glTexBuffer, "glTexBuffer");
    GL_PROC(glDrawArraysInstanced, "glDrawArraysInstanced");
    GL_PROC(glDrawElementsInstanced, "glDrawElementsInstanced");
    GL_PROC(glGetUniformBlockIndex, "glGetUniformBlockIndex");
    GL_PROC(glGetActiveUniformBlockName, "glGetActiveUniformBlockName");
    GL_PROC(glUniformBlockBinding, "glUniformBlockBinding");
    GL_PROC(glBindBufferRange, "glBindBufferRange");
    GL_PROC(glFenceSync, "glFenceSync");
    GL_PROC(glClientWaitSync, "glClientWaitSync");
    GL_PROC(glDeleteSync, "glDeleteSync");
    GL_PROC(glDrawElementsBaseVertex, "glDrawElementsBaseVertex");
    GL_PROC(glDrawElementsInstancedBaseVertex, "glDrawElementsInstancedBaseVertex");
    GL_PROC(glGenSamplers, "glGenSamplers");
    GL_PROC(glDeleteSamplers, "glDeleteSamplers");
    GL_PROC(glBindSampler, "glBindSampler");
    GL_PROC(glSamplerParameteri, "glSamplerParameteri");
    GL_PROC(glSamplerParameterf, "glSamplerParameterf");
    GL_PROC(glGenQueries, "glGenQueries");
    GL_PROC(glDeleteQueries, "glDeleteQueries");
    GL_PROC(glBeginQuery, "glBeginQuery");
    GL_PROC(glEndQuery, "glEndQuery");
    GL_PROC(glGetQueryObjectiv, "glGetQueryObjectiv");
    GL_PROC(glGetQueryObjecti64v, "glGetQueryObjecti64v");
    GL_PROC(glBufferStorage, "glBufferStorage");
    GL_PROC(glCreateBuffers, "glCreateBuffers");
    GL_PROC(glNamedBufferData, "glNamedBufferData");
    GL_PROC(glNamedBufferStorage, "glNamedBufferStorage");
    GL_PROC(glNamedBufferSubData, "glNamedBufferSubData");
    GL_PROC(glMapNamedBufferRange, "glMapNamedBufferRange");
    GL_PROC(glFlushMappedNamedBufferRange, "glFlushMappedNamedBufferRange");
    GL_PROC(glUnmapNamedBuffer, "glUnmapNamedBuffer");
    GL_PROC(glCopyNamedBufferSubData, "glCopyNamedBufferSubData");
    GL_PROC(glCreateFramebuffers, "glCreateFramebuffers");
    GL_PROC(glNamedFramebufferTexture, "glNamedFramebufferTexture");
    GL_PROC(glBlitNamedFramebuffer, "glBlitNamedFramebuffer");
    GL_PROC(glVertexAttribFormat, "glVertexAttribFormat");
    GL_PROC(glVertexAttribIFormat, "glVertexAttribIFormat");
    GL_PROC(glVertexAttribBinding, "glVertexAttribBinding");
    GL_PROC(glBindVertexBuffer, "glBindVertexBuffer");
    GL_PROC(glObjectLabel, "glObjectLabel");
    GL_PROC(glPushDebugGroup, "glPushDebugGroup");
    GL_PROC(glPopDebugGroup, "glPopDebugGroup");
    GL_PROC(glDebugMessageControl, "glDebugMessageControl");
    GL_PROC(glDebugMessageCallback, "glDebugMessageCallback");
    GL_PROC(glDebugMessageControlARB, "glDebugMessageControlARB");
    GL_PROC(glDebugMessageCallbackARB, "glDebugMessageCallbackARB");
    GL_PROC(glLabelObjectEXT, "glLabelObjectEXT");

    gl_loaded = 1;
    return 1;
}

void LIBMATTI_GL_Unload(void)
{
    // Java: LWJGL keeps the function table for the process lifetime; the port
    // resets the handles so a later Load() re-resolves them.
    gl_loaded = 0;
    gl_have_description = 0;
}

int LIBMATTI_GL_IsAvailable(void)
{
    return gl_lib != NULL;
}

const char *LIBMATTI_GL_Describe(void)
{
    return gl_have_description ? gl_description : NULL;
}

// ---------------------------------------------------------------------------
// The wrappers. Every function checks its entry point like LWJGL's generated
// calls (a NULL entry point means the driver lacks the extension) and no-ops
// with the documented fallback value.
// ---------------------------------------------------------------------------

void LIBMATTI_GL_glViewport(int x, int y, int width, int height)
{
    void (*fn)(int, int, int, int) = (void (*)(int, int, int, int)) p_glViewport;
    if (fn != NULL) fn(x, y, width, height);
}

void LIBMATTI_GL_glDepthFunc(unsigned int func)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glDepthFunc;
    if (fn != NULL) fn(func);
}

void LIBMATTI_GL_glDepthMask(unsigned char flag)
{
    void (*fn)(unsigned char) = (void (*)(unsigned char)) p_glDepthMask;
    if (fn != NULL) fn(flag);
}

void LIBMATTI_GL_glColorMask(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    void (*fn)(unsigned char, unsigned char, unsigned char, unsigned char) =
        (void (*)(unsigned char, unsigned char, unsigned char, unsigned char)) p_glColorMask;
    if (fn != NULL) fn(r, g, b, a);
}

void LIBMATTI_GL_glStencilFunc(unsigned int func, int ref, unsigned int mask)
{
    void (*fn)(unsigned int, int, unsigned int) = (void (*)(unsigned int, int, unsigned int)) p_glStencilFunc;
    if (fn != NULL) fn(func, ref, mask);
}

void LIBMATTI_GL_glStencilMask(unsigned int mask)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glStencilMask;
    if (fn != NULL) fn(mask);
}

void LIBMATTI_GL_glStencilOp(unsigned int sfail, unsigned int dpfail, unsigned int dppass)
{
    void (*fn)(unsigned int, unsigned int, unsigned int) =
        (void (*)(unsigned int, unsigned int, unsigned int)) p_glStencilOp;
    if (fn != NULL) fn(sfail, dpfail, dppass);
}

void LIBMATTI_GL_glClearStencil(int s)
{
    void (*fn)(int) = (void (*)(int)) p_glClearStencil;
    if (fn != NULL) fn(s);
}

void LIBMATTI_GL_glClearColor(float red, float green, float blue, float alpha)
{
    void (*fn)(float, float, float, float) = (void (*)(float, float, float, float)) p_glClearColor;
    if (fn != NULL) fn(red, green, blue, alpha);
}

void LIBMATTI_GL_glClear(unsigned int mask)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glClear;
    if (fn != NULL) fn(mask);
}

void LIBMATTI_GL_glLineWidth(float width)
{
    void (*fn)(float) = (void (*)(float)) p_glLineWidth;
    if (fn != NULL) fn(width);
}

void LIBMATTI_GL_glPolygonMode(unsigned int face, unsigned int mode)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glPolygonMode;
    if (fn != NULL) fn(face, mode);
}

void LIBMATTI_GL_glPolygonOffset(float factor, float units)
{
    void (*fn)(float, float) = (void (*)(float, float)) p_glPolygonOffset;
    if (fn != NULL) fn(factor, units);
}

void LIBMATTI_GL_glScissor(int x, int y, int width, int height)
{
    void (*fn)(int, int, int, int) = (void (*)(int, int, int, int)) p_glScissor;
    if (fn != NULL) fn(x, y, width, height);
}

unsigned int LIBMATTI_GL_glGetError(void)
{
    unsigned int (*fn)(void) = (unsigned int (*)(void)) p_glGetError;
    return fn != NULL ? fn() : LIBMATTI_GL_GL_NO_ERROR;
}

const char *LIBMATTI_GL_glGetString(unsigned int name)
{
    const char *(*fn)(unsigned int) = (const char *(*)(unsigned int)) p_glGetString;
    const char *value = fn != NULL ? fn(name) : NULL;
    // Java: GlDevice renders the "GL_VENDOR GL_RENDERER GL_VERSION" line during
    // startup. The calls need a current context (LWJGL documents that for
    // GLCapabilities.create()); the port caches them when the game makes them.
    if (value != NULL && (name == 7936 || name == 7937 || name == 7938)) // GL_VENDOR/GL_RENDERER/GL_VERSION
    {
        snprintf(gl_description, sizeof(gl_description), "%s", value);
        gl_have_description = 1;
    }
    return value;
}

int LIBMATTI_GL_glGetInteger(unsigned int pname)
{
    // Java: GL11.glGetInteger wraps glGetIntegerv with a one-element buffer.
    int value = 0;
    void (*fn)(unsigned int, int *) = (void (*)(unsigned int, int *)) p_glGetIntegerv;
    if (fn != NULL) fn(pname, &value);
    return value;
}

float LIBMATTI_GL_glGetFloat(unsigned int pname)
{
    // Java: GL11.glGetFloat wraps glGetFloatv with a one-element buffer.
    float value = 0.0f;
    void (*fn)(unsigned int, float *) = (void (*)(unsigned int, float *)) p_glGetFloatv;
    if (fn != NULL) fn(pname, &value);
    return value;
}

void LIBMATTI_GL_glGetFloatv(unsigned int pname, float *params)
{
    void (*fn)(unsigned int, float *) = (void (*)(unsigned int, float *)) p_glGetFloatv;
    if (fn != NULL) fn(pname, params);
}

void LIBMATTI_GL_glReadPixels(int x, int y, int width, int height, unsigned int format, unsigned int type, void *pixels)
{
    void (*fn)(int, int, int, int, unsigned int, unsigned int, void *) =
        (void (*)(int, int, int, int, unsigned int, unsigned int, void *)) p_glReadPixels;
    if (fn != NULL) fn(x, y, width, height, format, type, pixels);
}

void LIBMATTI_GL_glEnable(unsigned int cap)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glEnable;
    if (fn != NULL) fn(cap);
}

void LIBMATTI_GL_glDisable(unsigned int cap)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glDisable;
    if (fn != NULL) fn(cap);
}

void LIBMATTI_GL_glLogicOp(unsigned int opcode)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glLogicOp;
    if (fn != NULL) fn(opcode);
}

void LIBMATTI_GL_glDrawBuffer(unsigned int mode)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glDrawBuffer;
    if (fn != NULL) fn(mode);
}

void LIBMATTI_GL_glActiveTexture(unsigned int texture)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glActiveTexture;
    if (fn != NULL) fn(texture);
}

void LIBMATTI_GL_glBlendFuncSeparate(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha)
{
    void (*fn)(unsigned int, unsigned int, unsigned int, unsigned int) =
        (void (*)(unsigned int, unsigned int, unsigned int, unsigned int)) p_glBlendFuncSeparate;
    if (fn != NULL) fn(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void LIBMATTI_GL_glBlendEquationSeparate(unsigned int modeRGB, unsigned int modeAlpha)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glBlendEquationSeparate;
    if (fn != NULL) fn(modeRGB, modeAlpha);
}

void LIBMATTI_GL_glGenBuffers(int n, unsigned int *buffers)
{
    void (*fn)(int, unsigned int *) = (void (*)(int, unsigned int *)) p_glGenBuffers;
    if (fn != NULL) fn(n, buffers);
    else if (buffers != NULL) for (int i = 0; i < n; i++) buffers[i] = 0;
}

void LIBMATTI_GL_glDeleteBuffers(int n, const unsigned int *buffers)
{
    void (*fn)(int, const unsigned int *) = (void (*)(int, const unsigned int *)) p_glDeleteBuffers;
    if (fn != NULL) fn(n, buffers);
}

void LIBMATTI_GL_glBindBuffer(unsigned int target, unsigned int buffer)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glBindBuffer;
    if (fn != NULL) fn(target, buffer);
}

void LIBMATTI_GL_glBufferData(unsigned int target, long size, const void *data, unsigned int usage)
{
    void (*fn)(unsigned int, long, const void *, unsigned int) =
        (void (*)(unsigned int, long, const void *, unsigned int)) p_glBufferData;
    if (fn != NULL) fn(target, size, data, usage);
}

void LIBMATTI_GL_glBufferSubData(unsigned int target, long offset, long size, const void *data)
{
    void (*fn)(unsigned int, long, long, const void *) =
        (void (*)(unsigned int, long, long, const void *)) p_glBufferSubData;
    if (fn != NULL) fn(target, offset, size, data);
}

void *LIBMATTI_GL_glMapBufferRange(unsigned int target, long offset, long length, unsigned int access)
{
    void *(*fn)(unsigned int, long, long, unsigned int) =
        (void *(*)(unsigned int, long, long, unsigned int)) p_glMapBufferRange;
    return fn != NULL ? fn(target, offset, length, access) : NULL;
}

unsigned char LIBMATTI_GL_glUnmapBuffer(unsigned int target)
{
    unsigned char (*fn)(unsigned int) = (unsigned char (*)(unsigned int)) p_glUnmapBuffer;
    return fn != NULL ? fn(target) : LIBMATTI_GL_GL_FALSE;
}

void LIBMATTI_GL_glFlushMappedBufferRange(unsigned int target, long offset, long length)
{
    void (*fn)(unsigned int, long, long) = (void (*)(unsigned int, long, long)) p_glFlushMappedBufferRange;
    if (fn != NULL) fn(target, offset, length);
}

unsigned int LIBMATTI_GL_glCreateShader(unsigned int type)
{
    unsigned int (*fn)(unsigned int) = (unsigned int (*)(unsigned int)) p_glCreateShader;
    return fn != NULL ? fn(type) : 0;
}

void LIBMATTI_GL_glShaderSource(unsigned int shader, const char *source)
{
    // Java: GlStateManager.glShaderSource packs the source into a
    // one-element pointer array before the native call; the port does the
    // same in C.
    void (*fn)(unsigned int, int, const char **, const int *) =
        (void (*)(unsigned int, int, const char **, const int *)) p_glShaderSource;
    if (fn == NULL) return;
    const char *sources[1] = {source};
    int length = source != NULL ? (int) strlen(source) : 0;
    fn(shader, 1, sources, &length);
}

void LIBMATTI_GL_glCompileShader(unsigned int shader)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glCompileShader;
    if (fn != NULL) fn(shader);
}

void LIBMATTI_GL_glDeleteShader(unsigned int shader)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glDeleteShader;
    if (fn != NULL) fn(shader);
}

int LIBMATTI_GL_glGetShaderi(unsigned int shader, unsigned int pname)
{
    // Java: GL20.glGetShaderi wraps glGetShaderiv with a one-element buffer.
    int value = 0;
    void (*fn)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetShaderiv;
    if (fn != NULL) fn(shader, pname, &value);
    return value;
}

const char *LIBMATTI_GL_glGetShaderInfoLog(unsigned int shader)
{
    // Java: GL20.glGetShaderInfoLog wraps glGetShaderInfoLog with a sized
    // buffer; LWJGL sizes it from glGetShaderiv(GL_INFO_LOG_LENGTH). The port
    // returns the thread-local buffer GLDebug formats too.
    static char log[4096];
    log[0] = '\0';
    int length = 0;
    void (*fnIv)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetShaderiv;
    if (fnIv != NULL) fnIv(shader, 35716, &length); // GL_INFO_LOG_LENGTH
    if (length <= 0) return log;
    if (length > (int) sizeof(log) - 1) length = (int) sizeof(log) - 1;
    void (*fn)(unsigned int, int, int *, char *) =
        (void (*)(unsigned int, int, int *, char *)) p_glGetShaderInfoLog;
    if (fn != NULL) fn(shader, length, NULL, log);
    return log;
}

unsigned int LIBMATTI_GL_glCreateProgram(void)
{
    unsigned int (*fn)(void) = (unsigned int (*)(void)) p_glCreateProgram;
    return fn != NULL ? fn() : 0;
}

void LIBMATTI_GL_glDeleteProgram(unsigned int program)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glDeleteProgram;
    if (fn != NULL) fn(program);
}

void LIBMATTI_GL_glAttachShader(unsigned int program, unsigned int shader)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glAttachShader;
    if (fn != NULL) fn(program, shader);
}

void LIBMATTI_GL_glLinkProgram(unsigned int program)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glLinkProgram;
    if (fn != NULL) fn(program);
}

int LIBMATTI_GL_glGetProgrami(unsigned int program, unsigned int pname)
{
    // Java: GL20.glGetProgrami wraps glGetProgramiv with a one-element buffer.
    int value = 0;
    void (*fn)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetProgramiv;
    if (fn != NULL) fn(program, pname, &value);
    return value;
}

const char *LIBMATTI_GL_glGetProgramInfoLog(unsigned int program)
{
    static char log[4096];
    log[0] = '\0';
    int length = 0;
    void (*fnIv)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetProgramiv;
    if (fnIv != NULL) fnIv(program, 35716, &length); // GL_INFO_LOG_LENGTH
    if (length <= 0) return log;
    if (length > (int) sizeof(log) - 1) length = (int) sizeof(log) - 1;
    void (*fn)(unsigned int, int, int *, char *) =
        (void (*)(unsigned int, int, int *, char *)) p_glGetProgramInfoLog;
    if (fn != NULL) fn(program, length, NULL, log);
    return log;
}

void LIBMATTI_GL_glUseProgram(unsigned int program)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glUseProgram;
    if (fn != NULL) fn(program);
}

int LIBMATTI_GL_glGetUniformLocation(unsigned int program, const char *name)
{
    int (*fn)(unsigned int, const char *) = (int (*)(unsigned int, const char *)) p_glGetUniformLocation;
    return fn != NULL ? fn(program, name) : -1;
}

void LIBMATTI_GL_glGetUniformfv(unsigned int program, int location, float *params)
{
    void (*fn)(unsigned int, int, float *) = (void (*)(unsigned int, int, float *)) p_glGetUniformfv;
    if (fn != NULL) fn(program, location, params);
}

void LIBMATTI_GL_glUniform1i(int location, int v0)
{
    void (*fn)(int, int) = (void (*)(int, int)) p_glUniform1i;
    if (fn != NULL) fn(location, v0);
}

void LIBMATTI_GL_glUniform2f(int location, float v0, float v1)
{
    void (*fn)(int, float, float) = (void (*)(int, float, float)) p_glUniform2f;
    if (fn != NULL) fn(location, v0, v1);
}

void LIBMATTI_GL_glUniform3f(int location, float v0, float v1, float v2)
{
    void (*fn)(int, float, float, float) = (void (*)(int, float, float, float)) p_glUniform3f;
    if (fn != NULL) fn(location, v0, v1, v2);
}

void LIBMATTI_GL_glUniformMatrix4fv(int location, int transpose, const float *value)
{
    // The real signature is (location, count, transpose, value); the port's
    // wrapper takes (location, transpose, value) and uploads one matrix.
    void (*fn)(int, int, int, const float *) = (void (*)(int, int, int, const float *)) p_glUniformMatrix4fv;
    if (fn != NULL) fn(location, 1, transpose, value);
}

int LIBMATTI_GL_glGetShaderiv(unsigned int shader, unsigned int pname)
{
    int value = 0;
    void (*fn)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetShaderiv;
    if (fn != NULL) fn(shader, pname, &value);
    return value;
}

int LIBMATTI_GL_glGetProgramiv(unsigned int program, unsigned int pname)
{
    int value = 0;
    void (*fn)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetProgramiv;
    if (fn != NULL) fn(program, pname, &value);
    return value;
}

void LIBMATTI_GL_glUniform4f(int location, float v0, float v1, float v2, float v3)
{
    void (*fn)(int, float, float, float, float) =
        (void (*)(int, float, float, float, float)) p_glUniform4f;
    if (fn != NULL) fn(location, v0, v1, v2, v3);
}

void LIBMATTI_GL_glBindAttribLocation(unsigned int program, unsigned int index, const char *name)
{
    void (*fn)(unsigned int, unsigned int, const char *) =
        (void (*)(unsigned int, unsigned int, const char *)) p_glBindAttribLocation;
    if (fn != NULL) fn(program, index, name);
}

void LIBMATTI_GL_glEnableVertexAttribArray(unsigned int index)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glEnableVertexAttribArray;
    if (fn != NULL) fn(index);
}

void LIBMATTI_GL_glVertexAttribPointer(unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void *pointer)
{
    void (*fn)(unsigned int, int, unsigned int, unsigned char, int, const void *) =
        (void (*)(unsigned int, int, unsigned int, unsigned char, int, const void *)) p_glVertexAttribPointer;
    if (fn != NULL) fn(index, size, type, normalized, stride, pointer);
}

void LIBMATTI_GL_glVertexAttribIPointer(unsigned int index, int size, unsigned int type, int stride, const void *pointer)
{
    void (*fn)(unsigned int, int, unsigned int, int, const void *) =
        (void (*)(unsigned int, int, unsigned int, int, const void *)) p_glVertexAttribIPointer;
    if (fn != NULL) fn(index, size, type, stride, pointer);
}

void LIBMATTI_GL_glGenVertexArrays(int n, unsigned int *arrays)
{
    void (*fn)(int, unsigned int *) = (void (*)(int, unsigned int *)) p_glGenVertexArrays;
    if (fn != NULL) fn(n, arrays);
    else if (arrays != NULL) for (int i = 0; i < n; i++) arrays[i] = 0;
}

void LIBMATTI_GL_glDeleteVertexArrays(int n, const unsigned int *arrays)
{
    void (*fn)(int, const unsigned int *) = (void (*)(int, const unsigned int *)) p_glDeleteVertexArrays;
    if (fn != NULL) fn(n, arrays);
}

void LIBMATTI_GL_glBindVertexArray(unsigned int array)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glBindVertexArray;
    if (fn != NULL) fn(array);
}

void LIBMATTI_GL_glGenFramebuffers(int n, unsigned int *framebuffers)
{
    void (*fn)(int, unsigned int *) = (void (*)(int, unsigned int *)) p_glGenFramebuffers;
    if (fn != NULL) fn(n, framebuffers);
    else if (framebuffers != NULL) for (int i = 0; i < n; i++) framebuffers[i] = 0;
}

void LIBMATTI_GL_glDeleteFramebuffers(int n, const unsigned int *framebuffers)
{
    void (*fn)(int, const unsigned int *) = (void (*)(int, const unsigned int *)) p_glDeleteFramebuffers;
    if (fn != NULL) fn(n, framebuffers);
}

void LIBMATTI_GL_glBindFramebuffer(unsigned int target, unsigned int framebuffer)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glBindFramebuffer;
    if (fn != NULL) fn(target, framebuffer);
}

void LIBMATTI_GL_glFramebufferTexture2D(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level)
{
    void (*fn)(unsigned int, unsigned int, unsigned int, unsigned int, int) =
        (void (*)(unsigned int, unsigned int, unsigned int, unsigned int, int)) p_glFramebufferTexture2D;
    if (fn != NULL) fn(target, attachment, textarget, texture, level);
}

void LIBMATTI_GL_glGenRenderbuffers(int n, unsigned int *renderbuffers)
{
    void (*fn)(int, unsigned int *) = (void (*)(int, unsigned int *)) p_glGenRenderbuffers;
    if (fn != NULL) fn(n, renderbuffers);
}

void LIBMATTI_GL_glBindRenderbuffer(unsigned int target, unsigned int renderbuffer)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glBindRenderbuffer;
    if (fn != NULL) fn(target, renderbuffer);
}

void LIBMATTI_GL_glRenderbufferStorage(unsigned int target, unsigned int internalformat, int width, int height)
{
    void (*fn)(unsigned int, unsigned int, int, int) =
        (void (*)(unsigned int, unsigned int, int, int)) p_glRenderbufferStorage;
    if (fn != NULL) fn(target, internalformat, width, height);
}

void LIBMATTI_GL_glFramebufferRenderbuffer(unsigned int target, unsigned int attachment,
                                           unsigned int renderbuffertarget, unsigned int renderbuffer)
{
    void (*fn)(unsigned int, unsigned int, unsigned int, unsigned int) =
        (void (*)(unsigned int, unsigned int, unsigned int, unsigned int)) p_glFramebufferRenderbuffer;
    if (fn != NULL) fn(target, attachment, renderbuffertarget, renderbuffer);
}

unsigned int LIBMATTI_GL_glCheckFramebufferStatus(unsigned int target)
{
    unsigned int (*fn)(unsigned int) = (unsigned int (*)(unsigned int)) p_glCheckFramebufferStatus;
    return fn != NULL ? fn(target) : LIBMATTI_GL_GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
}

void LIBMATTI_GL_glBlitFramebuffer(int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter)
{
    void (*fn)(int, int, int, int, int, int, int, int, unsigned int, unsigned int) =
        (void (*)(int, int, int, int, int, int, int, int, unsigned int, unsigned int)) p_glBlitFramebuffer;
    if (fn != NULL) fn(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void LIBMATTI_GL_glGenTextures(int n, unsigned int *textures)
{
    void (*fn)(int, unsigned int *) = (void (*)(int, unsigned int *)) p_glGenTextures;
    if (fn != NULL) fn(n, textures);
    else if (textures != NULL) for (int i = 0; i < n; i++) textures[i] = 0;
}

void LIBMATTI_GL_glDeleteTextures(int n, const unsigned int *textures)
{
    void (*fn)(int, const unsigned int *) = (void (*)(int, const unsigned int *)) p_glDeleteTextures;
    if (fn != NULL) fn(n, textures);
}

void LIBMATTI_GL_glBindTexture(unsigned int target, unsigned int texture)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glBindTexture;
    if (fn != NULL) fn(target, texture);
}

void LIBMATTI_GL_glGetTexParameteriv(unsigned int target, unsigned int pname, int *params)
{
    void (*fn)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetTexParameteriv;
    if (fn != NULL) fn(target, pname, params);
}

void LIBMATTI_GL_glGenerateMipmap(unsigned int target)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glGenerateMipmap;
    if (fn != NULL) fn(target);
}

void LIBMATTI_GL_glTexImage2D(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels)
{
    void (*fn)(unsigned int, int, int, int, int, int, unsigned int, unsigned int, const void *) =
        (void (*)(unsigned int, int, int, int, int, int, unsigned int, unsigned int, const void *)) p_glTexImage2D;
    if (fn != NULL) fn(target, level, internalformat, width, height, border, format, type, pixels);
}

void LIBMATTI_GL_glTexSubImage2D(unsigned int target, int level, int xoffset, int yoffset, int width, int height, unsigned int format, unsigned int type, const void *pixels)
{
    void (*fn)(unsigned int, int, int, int, int, int, unsigned int, unsigned int, const void *) =
        (void (*)(unsigned int, int, int, int, int, int, unsigned int, unsigned int, const void *)) p_glTexSubImage2D;
    if (fn != NULL) fn(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void LIBMATTI_GL_glTexParameteri(unsigned int target, unsigned int pname, int param)
{
    void (*fn)(unsigned int, unsigned int, int) = (void (*)(unsigned int, unsigned int, int)) p_glTexParameteri;
    if (fn != NULL) fn(target, pname, param);
}

void LIBMATTI_GL_glTexParameterf(unsigned int target, unsigned int pname, float param)
{
    void (*fn)(unsigned int, unsigned int, float) = (void (*)(unsigned int, unsigned int, float)) p_glTexParameterf;
    if (fn != NULL) fn(target, pname, param);
}

int LIBMATTI_GL_glGetTexLevelParameteri(unsigned int target, int level, unsigned int pname)
{
    // Java: GL11.glGetTexLevelParameteri wraps glGetTexLevelParameteriv with a
    // one-element buffer.
    int value = 0;
    void (*fn)(unsigned int, int, unsigned int, int *) =
        (void (*)(unsigned int, int, unsigned int, int *)) p_glGetTexLevelParameteriv;
    if (fn != NULL) fn(target, level, pname, &value);
    return value;
}

void LIBMATTI_GL_glCopyBufferSubData(unsigned int readTarget, unsigned int writeTarget, long readOffset, long writeOffset, long size)
{
    void (*fn)(unsigned int, unsigned int, long, long, long) =
        (void (*)(unsigned int, unsigned int, long, long, long)) p_glCopyBufferSubData;
    if (fn != NULL) fn(readTarget, writeTarget, readOffset, writeOffset, size);
}

void LIBMATTI_GL_glTexBuffer(unsigned int target, unsigned int internalformat, unsigned int buffer)
{
    void (*fn)(unsigned int, unsigned int, unsigned int) =
        (void (*)(unsigned int, unsigned int, unsigned int)) p_glTexBuffer;
    if (fn != NULL) fn(target, internalformat, buffer);
}

void LIBMATTI_GL_glDrawArrays(unsigned int mode, int first, int count)
{
    void (*fn)(unsigned int, int, int) = (void (*)(unsigned int, int, int)) p_glDrawArrays;
    if (fn != NULL) fn(mode, first, count);
}

void LIBMATTI_GL_glDrawElements(unsigned int mode, int count, unsigned int type, const void *indices)
{
    void (*fn)(unsigned int, int, unsigned int, const void *) =
        (void (*)(unsigned int, int, unsigned int, const void *)) p_glDrawElements;
    if (fn != NULL) fn(mode, count, type, indices);
}

void LIBMATTI_GL_glPixelStorei(unsigned int pname, int param)
{
    void (*fn)(unsigned int, int) = (void (*)(unsigned int, int)) p_glPixelStorei;
    if (fn != NULL) fn(pname, param);
}

void LIBMATTI_GL_glDrawArraysInstanced(unsigned int mode, int first, int count, int instancecount)
{
    void (*fn)(unsigned int, int, int, int) =
        (void (*)(unsigned int, int, int, int)) p_glDrawArraysInstanced;
    if (fn != NULL) fn(mode, first, count, instancecount);
}

void LIBMATTI_GL_glDrawElementsInstanced(unsigned int mode, int count, unsigned int type, const void *indices, int instancecount)
{
    void (*fn)(unsigned int, int, unsigned int, const void *, int) =
        (void (*)(unsigned int, int, unsigned int, const void *, int)) p_glDrawElementsInstanced;
    if (fn != NULL) fn(mode, count, type, indices, instancecount);
}

int LIBMATTI_GL_glGetUniformBlockIndex(unsigned int program, const char *uniformBlockName)
{
    int (*fn)(unsigned int, const char *) = (int (*)(unsigned int, const char *)) p_glGetUniformBlockIndex;
    return fn != NULL ? fn(program, uniformBlockName) : -1;
}

void LIBMATTI_GL_glGetActiveUniformBlockName(unsigned int program, unsigned int uniformBlockIndex, char *uniformBlockName)
{
    // Java: GL31.glGetActiveUniformBlockName writes up to the buffer size; the
    // game sizes the array from GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH. The
    // port passes a 256-sized buffer like the generated LWJGL call.
    void (*fn)(unsigned int, unsigned int, int, int *, char *) =
        (void (*)(unsigned int, unsigned int, int, int *, char *)) p_glGetActiveUniformBlockName;
    if (fn == NULL || uniformBlockName == NULL) return;
    int length = 0;
    fn(program, uniformBlockIndex, 256, &length, uniformBlockName);
}

void LIBMATTI_GL_glUniformBlockBinding(unsigned int program, unsigned int uniformBlockIndex, unsigned int uniformBlockBinding)
{
    void (*fn)(unsigned int, unsigned int, unsigned int) =
        (void (*)(unsigned int, unsigned int, unsigned int)) p_glUniformBlockBinding;
    if (fn != NULL) fn(program, uniformBlockIndex, uniformBlockBinding);
}

void LIBMATTI_GL_glBindBufferRange(unsigned int target, unsigned int index, unsigned int buffer, long offset, long size)
{
    void (*fn)(unsigned int, unsigned int, unsigned int, long, long) =
        (void (*)(unsigned int, unsigned int, unsigned int, long, long)) p_glBindBufferRange;
    if (fn != NULL) fn(target, index, buffer, offset, size);
}

void *LIBMATTI_GL_glFenceSync(unsigned int condition, unsigned int flags)
{
    void *(*fn)(unsigned int, unsigned int) = (void *(*)(unsigned int, unsigned int)) p_glFenceSync;
    return fn != NULL ? fn(condition, flags) : NULL;
}

unsigned int LIBMATTI_GL_glClientWaitSync(void *sync, unsigned int flags, unsigned long long timeout)
{
    // Java: GL32.glClientWaitSync returns the GLenum status (GLint in the C API).
    unsigned int (*fn)(void *, unsigned int, unsigned long long) =
        (unsigned int (*)(void *, unsigned int, unsigned long long)) p_glClientWaitSync;
    return fn != NULL ? fn(sync, flags, timeout) : LIBMATTI_GL_GL_WAIT_FAILED;
}

void LIBMATTI_GL_glDeleteSync(void *sync)
{
    void (*fn)(void *) = (void (*)(void *)) p_glDeleteSync;
    if (fn != NULL) fn(sync);
}

void LIBMATTI_GL_glDrawElementsBaseVertex(unsigned int mode, int count, unsigned int type, const void *indices, int basevertex)
{
    void (*fn)(unsigned int, int, unsigned int, const void *, int) =
        (void (*)(unsigned int, int, unsigned int, const void *, int)) p_glDrawElementsBaseVertex;
    if (fn != NULL) fn(mode, count, type, indices, basevertex);
}

void LIBMATTI_GL_glDrawElementsInstancedBaseVertex(unsigned int mode, int count, unsigned int type, const void *indices, int instancecount, int basevertex)
{
    void (*fn)(unsigned int, int, unsigned int, const void *, int, int) =
        (void (*)(unsigned int, int, unsigned int, const void *, int, int)) p_glDrawElementsInstancedBaseVertex;
    if (fn != NULL) fn(mode, count, type, indices, instancecount, basevertex);
}

void LIBMATTI_GL_glGenSamplers(int n, unsigned int *samplers)
{
    void (*fn)(int, unsigned int *) = (void (*)(int, unsigned int *)) p_glGenSamplers;
    if (fn != NULL) fn(n, samplers);
    else if (samplers != NULL) for (int i = 0; i < n; i++) samplers[i] = 0;
}

void LIBMATTI_GL_glDeleteSamplers(int n, const unsigned int *samplers)
{
    void (*fn)(int, const unsigned int *) = (void (*)(int, const unsigned int *)) p_glDeleteSamplers;
    if (fn != NULL) fn(n, samplers);
}

void LIBMATTI_GL_glBindSampler(unsigned int unit, unsigned int sampler)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glBindSampler;
    if (fn != NULL) fn(unit, sampler);
}

void LIBMATTI_GL_glSamplerParameteri(unsigned int sampler, unsigned int pname, int param)
{
    void (*fn)(unsigned int, unsigned int, int) = (void (*)(unsigned int, unsigned int, int)) p_glSamplerParameteri;
    if (fn != NULL) fn(sampler, pname, param);
}

void LIBMATTI_GL_glSamplerParameterf(unsigned int sampler, unsigned int pname, float param)
{
    void (*fn)(unsigned int, unsigned int, float) = (void (*)(unsigned int, unsigned int, float)) p_glSamplerParameterf;
    if (fn != NULL) fn(sampler, pname, param);
}

void LIBMATTI_GL_glGenQueries(int n, unsigned int *ids)
{
    void (*fn)(int, unsigned int *) = (void (*)(int, unsigned int *)) p_glGenQueries;
    if (fn != NULL) fn(n, ids);
    else if (ids != NULL) for (int i = 0; i < n; i++) ids[i] = 0;
}

void LIBMATTI_GL_glDeleteQueries(int n, const unsigned int *ids)
{
    void (*fn)(int, const unsigned int *) = (void (*)(int, const unsigned int *)) p_glDeleteQueries;
    if (fn != NULL) fn(n, ids);
}

void LIBMATTI_GL_glBeginQuery(unsigned int target, unsigned int id)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glBeginQuery;
    if (fn != NULL) fn(target, id);
}

void LIBMATTI_GL_glEndQuery(unsigned int target)
{
    void (*fn)(unsigned int) = (void (*)(unsigned int)) p_glEndQuery;
    if (fn != NULL) fn(target);
}

int LIBMATTI_GL_glGetQueryObjecti(unsigned int id, unsigned int pname)
{
    // Java: ARBTimerQuery.glGetQueryObjecti wraps glGetQueryObjectiv with a
    // one-element buffer.
    int value = 0;
    void (*fn)(unsigned int, unsigned int, int *) = (void (*)(unsigned int, unsigned int, int *)) p_glGetQueryObjectiv;
    if (fn != NULL) fn(id, pname, &value);
    return value;
}

long long LIBMATTI_GL_glGetQueryObjecti64(unsigned int id, unsigned int pname)
{
    // Java: ARBTimerQuery.glGetQueryObjecti64 wraps glGetQueryObjecti64v with
    // a one-element buffer.
    long long value = 0;
    void (*fn)(unsigned int, unsigned int, long long *) =
        (void (*)(unsigned int, unsigned int, long long *)) p_glGetQueryObjecti64v;
    if (fn != NULL) fn(id, pname, &value);
    return value;
}

void LIBMATTI_GL_glBufferStorage(unsigned int target, long size, const void *data, unsigned int flags)
{
    void (*fn)(unsigned int, long, const void *, unsigned int) =
        (void (*)(unsigned int, long, const void *, unsigned int)) p_glBufferStorage;
    if (fn != NULL) fn(target, size, data, flags);
}

unsigned int LIBMATTI_GL_glCreateBuffers(void)
{
    unsigned int (*fn)(void) = (unsigned int (*)(void)) p_glCreateBuffers;
    return fn != NULL ? fn() : 0;
}

void LIBMATTI_GL_glNamedBufferData(unsigned int buffer, long size, const void *data, unsigned int usage)
{
    void (*fn)(unsigned int, long, const void *, unsigned int) =
        (void (*)(unsigned int, long, const void *, unsigned int)) p_glNamedBufferData;
    if (fn != NULL) fn(buffer, size, data, usage);
}

void LIBMATTI_GL_glNamedBufferStorage(unsigned int buffer, long size, const void *data, unsigned int flags)
{
    void (*fn)(unsigned int, long, const void *, unsigned int) =
        (void (*)(unsigned int, long, const void *, unsigned int)) p_glNamedBufferStorage;
    if (fn != NULL) fn(buffer, size, data, flags);
}

void LIBMATTI_GL_glNamedBufferSubData(unsigned int buffer, long offset, long size, const void *data)
{
    void (*fn)(unsigned int, long, long, const void *) =
        (void (*)(unsigned int, long, long, const void *)) p_glNamedBufferSubData;
    if (fn != NULL) fn(buffer, offset, size, data);
}

void *LIBMATTI_GL_glMapNamedBufferRange(unsigned int buffer, long offset, long length, unsigned int access)
{
    void *(*fn)(unsigned int, long, long, unsigned int) =
        (void *(*)(unsigned int, long, long, unsigned int)) p_glMapNamedBufferRange;
    return fn != NULL ? fn(buffer, offset, length, access) : NULL;
}

void LIBMATTI_GL_glFlushMappedNamedBufferRange(unsigned int buffer, long offset, long length)
{
    void (*fn)(unsigned int, long, long) = (void (*)(unsigned int, long, long)) p_glFlushMappedNamedBufferRange;
    if (fn != NULL) fn(buffer, offset, length);
}

unsigned char LIBMATTI_GL_glUnmapNamedBuffer(unsigned int buffer)
{
    unsigned char (*fn)(unsigned int) = (unsigned char (*)(unsigned int)) p_glUnmapNamedBuffer;
    return fn != NULL ? fn(buffer) : LIBMATTI_GL_GL_FALSE;
}

void LIBMATTI_GL_glCopyNamedBufferSubData(unsigned int readBuffer, unsigned int writeBuffer, long readOffset, long writeOffset, long size)
{
    void (*fn)(unsigned int, unsigned int, long, long, long) =
        (void (*)(unsigned int, unsigned int, long, long, long)) p_glCopyNamedBufferSubData;
    if (fn != NULL) fn(readBuffer, writeBuffer, readOffset, writeOffset, size);
}

unsigned int LIBMATTI_GL_glCreateFramebuffers(void)
{
    unsigned int (*fn)(void) = (unsigned int (*)(void)) p_glCreateFramebuffers;
    return fn != NULL ? fn() : 0;
}

void LIBMATTI_GL_glNamedFramebufferTexture(unsigned int framebuffer, unsigned int attachment, unsigned int texture, int level)
{
    void (*fn)(unsigned int, unsigned int, unsigned int, int) =
        (void (*)(unsigned int, unsigned int, unsigned int, int)) p_glNamedFramebufferTexture;
    if (fn != NULL) fn(framebuffer, attachment, texture, level);
}

void LIBMATTI_GL_glBlitNamedFramebuffer(unsigned int readFramebuffer, unsigned int drawFramebuffer, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter)
{
    void (*fn)(unsigned int, unsigned int, int, int, int, int, int, int, int, int, unsigned int, unsigned int) =
        (void (*)(unsigned int, unsigned int, int, int, int, int, int, int, int, int, unsigned int, unsigned int)) p_glBlitNamedFramebuffer;
    if (fn != NULL) fn(readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void LIBMATTI_GL_glVertexAttribFormat(unsigned int attribindex, int size, unsigned int type, unsigned char normalized, unsigned int relativeoffset)
{
    void (*fn)(unsigned int, int, unsigned int, unsigned char, unsigned int) =
        (void (*)(unsigned int, int, unsigned int, unsigned char, unsigned int)) p_glVertexAttribFormat;
    if (fn != NULL) fn(attribindex, size, type, normalized, relativeoffset);
}

void LIBMATTI_GL_glVertexAttribIFormat(unsigned int attribindex, int size, unsigned int type, unsigned int relativeoffset)
{
    void (*fn)(unsigned int, int, unsigned int, unsigned int) =
        (void (*)(unsigned int, int, unsigned int, unsigned int)) p_glVertexAttribIFormat;
    if (fn != NULL) fn(attribindex, size, type, relativeoffset);
}

void LIBMATTI_GL_glVertexAttribBinding(unsigned int attribindex, unsigned int bindingindex)
{
    void (*fn)(unsigned int, unsigned int) = (void (*)(unsigned int, unsigned int)) p_glVertexAttribBinding;
    if (fn != NULL) fn(attribindex, bindingindex);
}

void LIBMATTI_GL_glBindVertexBuffer(unsigned int bindingindex, unsigned int buffer, long offset, int stride)
{
    void (*fn)(unsigned int, unsigned int, long, int) =
        (void (*)(unsigned int, unsigned int, long, int)) p_glBindVertexBuffer;
    if (fn != NULL) fn(bindingindex, buffer, offset, stride);
}

void LIBMATTI_GL_glObjectLabel(unsigned int identifier, unsigned int name, const char *label)
{
    // Java: KHRDebug.glObjectLabel takes (identifier, name, length, label) and
    // LWJGL measures the string; the port passes the measured length like the
    // generated call.
    void (*fn)(unsigned int, unsigned int, int, const char *) =
        (void (*)(unsigned int, unsigned int, int, const char *)) p_glObjectLabel;
    if (fn != NULL) fn(identifier, name, label != NULL ? (int) strlen(label) : 0, label);
}

void LIBMATTI_GL_glPushDebugGroup(unsigned int source, unsigned int id, const char *message)
{
    // Java: KHRDebug.glPushDebugGroup takes (source, id, length, message).
    void (*fn)(unsigned int, unsigned int, int, const char *) =
        (void (*)(unsigned int, unsigned int, int, const char *)) p_glPushDebugGroup;
    if (fn != NULL) fn(source, id, message != NULL ? (int) strlen(message) : 0, message);
}

void LIBMATTI_GL_glPopDebugGroup(void)
{
    void (*fn)(void) = (void (*)(void)) p_glPopDebugGroup;
    if (fn != NULL) fn();
}

void LIBMATTI_GL_glDebugMessageControl(unsigned int source, unsigned int type, unsigned int severity, int count, const unsigned int *ids, unsigned char enabled)
{
    void (*fn)(unsigned int, unsigned int, unsigned int, int, const unsigned int *, unsigned char) =
        (void (*)(unsigned int, unsigned int, unsigned int, int, const unsigned int *, unsigned char)) p_glDebugMessageControl;
    if (fn != NULL) fn(source, type, severity, count, ids, enabled);
}

void LIBMATTI_GL_glDebugMessageCallback(LIBMATTI_GL_DebugMessageCallbackI callback, const void *userParam)
{
    void (*fn)(LIBMATTI_GL_DebugMessageCallbackI, const void *) =
        (void (*)(LIBMATTI_GL_DebugMessageCallbackI, const void *)) p_glDebugMessageCallback;
    if (fn != NULL) fn(callback, userParam);
}

void LIBMATTI_GL_glDebugMessageControlARB(unsigned int source, unsigned int type, unsigned int severity, int count, const unsigned int *ids, unsigned char enabled)
{
    void (*fn)(unsigned int, unsigned int, unsigned int, int, const unsigned int *, unsigned char) =
        (void (*)(unsigned int, unsigned int, unsigned int, int, const unsigned int *, unsigned char)) p_glDebugMessageControlARB;
    if (fn != NULL) fn(source, type, severity, count, ids, enabled);
}

void LIBMATTI_GL_glDebugMessageCallbackARB(LIBMATTI_GL_DebugMessageCallbackI callback, const void *userParam)
{
    void (*fn)(LIBMATTI_GL_DebugMessageCallbackI, const void *) =
        (void (*)(LIBMATTI_GL_DebugMessageCallbackI, const void *)) p_glDebugMessageCallbackARB;
    if (fn != NULL) fn(callback, userParam);
}

void LIBMATTI_GL_glLabelObjectEXT(unsigned int type, unsigned int object, const char *label)
{
    // Java: EXTDebugLabel.glLabelObjectEXT takes (type, object, length, label).
    void (*fn)(unsigned int, unsigned int, int, const char *) =
        (void (*)(unsigned int, unsigned int, int, const char *)) p_glLabelObjectEXT;
    if (fn != NULL) fn(type, object, label != NULL ? (int) strlen(label) : 0, label);
}

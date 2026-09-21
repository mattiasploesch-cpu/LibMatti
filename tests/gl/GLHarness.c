// GL binding harness: checks the constant set against the GL specification
// values, the dlopen availability path (with and without a driver present)
// and the no-driver fallbacks. No display is needed - every call degrades to
// the documented fallback value like LWJGL's generated calls.

#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"

#include <stdio.h>
#include <stdlib.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                                              \
    do                                                                                           \
    {                                                                                            \
        checks++;                                                                                \
        if (!(cond))                                                                             \
        {                                                                                        \
            failures++;                                                                          \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                               \
        }                                                                                        \
    } while (0)

// Java: GlConst values are the GL specification values; spot-check the ranges
// the game depends on (blend factors, texture filters, buffer usage, FBO).
static void test_constants(void)
{
    CHECK(LIBMATTI_GL_GL_LESS == 513);
    CHECK(LIBMATTI_GL_GL_LEQUAL == 515);
    CHECK(LIBMATTI_GL_GL_SRC_ALPHA == 770 && LIBMATTI_GL_GL_ONE_MINUS_SRC_ALPHA == 771);
    CHECK(LIBMATTI_GL_GL_NEAREST == 9728 && LIBMATTI_GL_GL_LINEAR == 9729);
    CHECK(LIBMATTI_GL_GL_CLAMP_TO_EDGE == 33071 && LIBMATTI_GL_GL_REPEAT == 10497);
    CHECK(LIBMATTI_GL_GL_TEXTURE0 == 33984);
    CHECK(LIBMATTI_GL_GL_ARRAY_BUFFER == 34962 && LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER == 34963);
    CHECK(LIBMATTI_GL_GL_STATIC_DRAW == 35044 && LIBMATTI_GL_GL_STREAM_DRAW == 35040);
    CHECK(LIBMATTI_GL_GL_FRAMEBUFFER == 36160 && LIBMATTI_GL_GL_RENDERBUFFER == 36161);
    CHECK(LIBMATTI_GL_GL_COLOR_ATTACHMENT0 == 36064 && LIBMATTI_GL_GL_DEPTH_ATTACHMENT == 36096);
    CHECK(LIBMATTI_GL_GL_FRAMEBUFFER_COMPLETE == 36053);
    CHECK(LIBMATTI_GL_GL_COMPILE_STATUS == 35713 && LIBMATTI_GL_GL_LINK_STATUS == 35714);
    CHECK(LIBMATTI_GL_GL_VERTEX_SHADER == 35633 && LIBMATTI_GL_GL_FRAGMENT_SHADER == 35632);
    CHECK(LIBMATTI_GL_GL_RGBA8 == 32856 && LIBMATTI_GL_GL_DEPTH_COMPONENT24 == 33190);
    CHECK(LIBMATTI_GL_GL_MAP_READ_BIT == 1 && LIBMATTI_GL_GL_MAP_WRITE_BIT == 2);
    CHECK(LIBMATTI_GL_GL_SYNC_GPU_COMMANDS_COMPLETE == 37143);
    CHECK(LIBMATTI_GL_GL_TIMEOUT_EXPIRED == 37147 && LIBMATTI_GL_GL_WAIT_FAILED == 37149);
    CHECK(LIBMATTI_GL_GL_TIME_ELAPSED == 35007);
    CHECK(LIBMATTI_GL_GL_MAX_TEXTURE_SIZE == 3379);
    CHECK(LIBMATTI_GL_GL_TEXTURE_WIDTH == 4096);
    CHECK(LIBMATTI_GL_GL_OUT_OF_MEMORY == 1285);
    CHECK(LIBMATTI_GL_GL_FALSE == 0 && LIBMATTI_GL_GL_TRUE == 1);
}

// Java: without a made-current context LWJGL throws; the port degrades to the
// documented fallback values instead (0 ids, GL_FALSE, NULL strings), which is
// the behaviour DisplayWindow sees when the driver library is absent.
static void test_no_driver_fallbacks(void)
{
    LIBMATTI_GL_Unload();

    unsigned int id = 12345;
    LIBMATTI_GL_glGenBuffers(1, &id);
    CHECK(id == 0);
    id = 12345;
    LIBMATTI_GL_glGenTextures(1, &id);
    CHECK(id == 0);
    id = 12345;
    LIBMATTI_GL_glGenVertexArrays(1, &id);
    CHECK(id == 0);
    id = 12345;
    LIBMATTI_GL_glGenFramebuffers(1, &id);
    CHECK(id == 0);
    id = 12345;
    LIBMATTI_GL_glGenSamplers(1, &id);
    CHECK(id == 0);
    id = 12345;
    LIBMATTI_GL_glGenQueries(1, &id);
    CHECK(id == 0);

    CHECK(LIBMATTI_GL_glGetError() == LIBMATTI_GL_GL_NO_ERROR);
    CHECK(LIBMATTI_GL_glGetString(7937) == NULL); // GL_RENDERER
    CHECK(LIBMATTI_GL_glGetInteger(LIBMATTI_GL_GL_MAX_TEXTURE_SIZE) == 0);
    CHECK(LIBMATTI_GL_glGetUniformLocation(1, "projection") == -1);
    CHECK(LIBMATTI_GL_glGetUniformBlockIndex(1, "RenderGlobals") == -1);
    CHECK(LIBMATTI_GL_glMapBufferRange(34962, 0, 16, 1) == NULL);
    CHECK(LIBMATTI_GL_glUnmapBuffer(34962) == LIBMATTI_GL_GL_FALSE);
    CHECK(LIBMATTI_GL_glFenceSync(37143, 0) == NULL);
    CHECK(LIBMATTI_GL_glClientWaitSync(NULL, 0, 0) == LIBMATTI_GL_GL_WAIT_FAILED);
    CHECK(LIBMATTI_GL_glCreateShader(LIBMATTI_GL_GL_VERTEX_SHADER) == 0);
    CHECK(LIBMATTI_GL_glCreateProgram() == 0);
    CHECK(LIBMATTI_GL_glGetShaderInfoLog(1)[0] == '\0');
    CHECK(LIBMATTI_GL_glGetProgramInfoLog(1)[0] == '\0');

    // The description is only available after the game made glGetString calls.
    CHECK(LIBMATTI_GL_Describe() == NULL);
    LIBMATTI_GL_Unload();
}

static void test_load(void)
{
    int loaded = LIBMATTI_GL_Load();
    if (!loaded)
    {
        printf("SKIP %s:%d: no GL driver library present\n", __FILE__, __LINE__);
        return;
    }

    CHECK(LIBMATTI_GL_IsAvailable() == 1);

    // Java: GL functions without a made-current context crash in the Mesa
    // dispatch stubs (LWJGL documents a current context as required for every
    // GL call), so the harness only verifies that the entry points resolved.
    CHECK(LIBMATTI_GL_Describe() == NULL);

    LIBMATTI_GL_Unload();
}

int main(void)
{
    test_constants();
    test_no_driver_fallbacks();
    test_load();

    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}

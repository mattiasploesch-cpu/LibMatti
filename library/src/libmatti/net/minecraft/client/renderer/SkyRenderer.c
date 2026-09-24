// Port of net.minecraft.client.renderer.SkyRenderer (implementation).
//
// The geometry builders mirror the Java lines: the sky disc fan, the
// sunrise/sunset fan, the sun quad, the eight moon phase quads and the 1500
// stars over RandomSource(10842L). Java draws each through its own render
// pass with the DynamicTransforms model-view stack; the port keeps one
// shader per pass kind (sky disc/celestial/stars/sunrise) and sets the
// model-view matrix + tint as uniforms.

#include "libmatti/net/minecraft/client/renderer/SkyRenderer.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/vertex/BufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/DefaultVertexFormat.h"
#include "libmatti/com/mojang/blaze3d/vertex/MeshData.h"
#include "libmatti/java/util/Random.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlasSprite.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/net/minecraft/world/level/MoonPhase.h"
#include "libmatti/org/joml/Matrix3f.h"
#include "libmatti/org/joml/Vector3f.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The vertex buffer views: a GL VBO plus the vertex count the pass draws
// (Java: GpuBuffer + the draw call's count).
typedef struct SkyBuffer
{
    unsigned int vbo;
    int vertexCount;
    int indexCount; // the quads' element count (0 for the fans/drawArrays)
} SkyBuffer;

struct LIBMATTI_MC_SkyRenderer
{
    // Java: private final TextureAtlas celestialsAtlas
    void *celestialsAtlas; // LIBMATTI_MC_TextureAtlas*

    // Java: private final GpuBuffer starBuffer, topSkyBuffer, bottomSkyBuffer,
    //       sunBuffer, moonBuffer, sunriseBuffer
    SkyBuffer starBuffer;
    SkyBuffer topSkyBuffer;
    SkyBuffer bottomSkyBuffer;
    SkyBuffer sunBuffer;
    SkyBuffer moonBuffer;
    SkyBuffer sunriseBuffer;

    // Java: private int starIndexCount
    int starIndexCount;

    // The pass programs (Java: RenderPipelines.SKY / CELESTIAL / STARS /
    // SUNRISE_SUNSET). 0 until the first draw with a live context.
    unsigned int skyProgram;
    unsigned int celestialProgram;
    unsigned int starsProgram;
    unsigned int sunriseProgram;

    // The quad index buffer (Java: RenderSystem.getSequentialBuffer(QUADS)).
    unsigned int quadIbo;
    int quadIboCapacity; // in quads
};

// Java: DefaultVertexFormat.POSITION's vertex size
#define POS_VERT_SIZE 12
// Java: DefaultVertexFormat.POSITION_COLOR's vertex size
#define POS_COLOR_VERT_SIZE 20
// Java: DefaultVertexFormat.POSITION_TEX's vertex size
#define POS_TEX_VERT_SIZE 20

// The fixed page UV for the sun sprite: the port's celestials page is the
// 96x96 grid with the sun in cell (0,0) -> the quad covers the full cell.
static void celestial_quad_uv(float u0, float v0, float u1, float v1, float *out)
{
    out[0] = u0; out[1] = v0;
    out[2] = u1; out[3] = v0;
    out[4] = u1; out[5] = v1;
    out[6] = u0; out[7] = v1;
}

// Java: private static GpuBuffer buildCelestialQuad(String, TextureAtlasSprite)
// - the 4-vertex quad over (-1..1, y=0, -1..1) with the sprite UVs, uploaded
// through the device (the port: one static VBO).
static int build_celestial_quad(unsigned int *outVbo, const float *uv)
{
    float vertices[4 * POS_TEX_VERT_SIZE / 4]; // 4 x (pos 3f + uv 2f)
    const float positions[4][3] = {
        {-1.0f, 0.0f, -1.0f}, {1.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 1.0f}};
    const float quadUv[4][2] = {{uv[0], uv[1]}, {uv[2], uv[3]}, {uv[4], uv[5]}, {uv[6], uv[7]}};
    for (int i = 0; i < 4; i++)
    {
        vertices[i * 5 + 0] = positions[i][0];
        vertices[i * 5 + 1] = positions[i][1];
        vertices[i * 5 + 2] = positions[i][2];
        vertices[i * 5 + 3] = quadUv[i][0];
        vertices[i * 5 + 4] = quadUv[i][1];
    }
    unsigned int vbos[1];
    LIBMATTI_GL_glGenBuffers(1, vbos);
    if (vbos[0] == 0)
        return 0;
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbos[0]);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, sizeof(vertices), vertices, LIBMATTI_GL_GL_STATIC_DRAW);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    *outVbo = vbos[0];
    return 1;
}

// Java: private static GpuBuffer buildSunQuad(TextureAtlas)
static int build_sun_quad(SkyBuffer *buffer, const LIBMATTI_MC_TextureAtlas *atlas)
{
    LIBMATTI_MC_Identifier *sun = LIBMATTI_MC_Identifier_WithDefaultNamespace("environment/sun");
    LIBMATTI_MC_TextureAtlasSprite *sprite = LIBMATTI_MC_TextureAtlas_GetSprite(atlas, sun);
    LIBMATTI_MC_Identifier_Free(sun);
    if (sprite == NULL)
        return 0;
    // Java: the sprite UVs (getU0..getV1 over the atlas page).
    float u0 = (float) sprite->x / (float) atlas->width;
    float v0 = (float) sprite->y / (float) atlas->height;
    float u1 = (float) (sprite->x + sprite->width) / (float) atlas->width;
    float v1 = (float) (sprite->y + sprite->height) / (float) atlas->height;
    float uv[8];
    celestial_quad_uv(u0, v0, u1, v1, uv);
    buffer->indexCount = 6;
    return build_celestial_quad(&buffer->vbo, uv);
}

// Java: private static GpuBuffer buildMoonPhases(TextureAtlas) - one quad per
// MoonPhase, concatenated into the buffer; the draw offsets by index * 4.
static int build_moon_phases(SkyBuffer *buffer, const LIBMATTI_MC_TextureAtlas *atlas)
{
    float vertices[LIBMATTI_MC_MoonPhase_COUNT * 4 * 5];
    const float positions[4][3] = {
        {-1.0f, 0.0f, -1.0f}, {1.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 1.0f}};
    for (int p = 0; p < LIBMATTI_MC_MoonPhase_COUNT; p++)
    {
        char path[64];
        snprintf(path, sizeof(path), "environment/moon/%s", LIBMATTI_MC_MoonPhase_GetSerializedName(
                                                          (LIBMATTI_MC_MoonPhase) p));
        LIBMATTI_MC_Identifier *spriteId = LIBMATTI_MC_Identifier_WithDefaultNamespace(path);
        LIBMATTI_MC_TextureAtlasSprite *sprite = LIBMATTI_MC_TextureAtlas_GetSprite(atlas, spriteId);
        LIBMATTI_MC_Identifier_Free(spriteId);
        if (sprite == NULL)
            return 0;
        float u0 = (float) sprite->x / (float) atlas->width;
        float v0 = (float) sprite->y / (float) atlas->height;
        float u1 = (float) (sprite->x + sprite->width) / (float) atlas->width;
        float v1 = (float) (sprite->y + sprite->height) / (float) atlas->height;
        // Java's phase quad flips u/v: v1 comes first, then v0.
        const float quadUv[4][2] = {{u1, v1}, {u0, v1}, {u0, v0}, {u1, v0}};
        for (int i = 0; i < 4; i++)
        {
            int v = p * 4 + i;
            vertices[v * 5 + 0] = positions[i][0];
            vertices[v * 5 + 1] = positions[i][1];
            vertices[v * 5 + 2] = positions[i][2];
            vertices[v * 5 + 3] = quadUv[i][0];
            vertices[v * 5 + 4] = quadUv[i][1];
        }
    }
    unsigned int vbos[1];
    LIBMATTI_GL_glGenBuffers(1, vbos);
    if (vbos[0] == 0)
        return 0;
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbos[0]);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, sizeof(vertices), vertices, LIBMATTI_GL_GL_STATIC_DRAW);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    buffer->vbo = vbos[0];
    buffer->indexCount = LIBMATTI_MC_MoonPhase_COUNT * 6;
    return 1;
}

// Java: private GpuBuffer buildStars() - the 1500 candidate loop over
// RandomSource.create(10842L); rejected candidates still consume their RNG
// draws, exactly like the Java flow.
static int build_stars(LIBMATTI_MC_SkyRenderer *renderer)
{
    LIBMATTI_JU_Random random;
    LIBMATTI_JU_Random_SetSeed(&random, 10842UL);
    const float radius = 100.0f;

    // Java: ByteBufferBuilder.exactlySized(POSITION.size * 1500 * 4)
    float *vertices = malloc(sizeof(float) * 5 * 1500 * 4);
    if (vertices == NULL)
        return 0;
    int vertexCount = 0;

    for (int i = 0; i < 1500; i++)
    {
        float f1 = LIBMATTI_JU_Random_NextFloat(&random) * 2.0f - 1.0f;
        float f2 = LIBMATTI_JU_Random_NextFloat(&random) * 2.0f - 1.0f;
        float f3 = LIBMATTI_JU_Random_NextFloat(&random) * 2.0f - 1.0f;
        float f4 = 0.15f + LIBMATTI_JU_Random_NextFloat(&random) * 0.1f;
        float f5sq = (float) LIBMATTI_MC_Mth_LengthSquared3(f1, f2, f3);
        if (f5sq <= 0.010000001f || f5sq >= 1.0f)
            continue;
        LIBMATTI_JOML_Vector3f star = {f1, f2, f3};
        LIBMATTI_JOML_Vector3f_Normalize(&star);
        LIBMATTI_JOML_Vector3f_MulScalar(&star, radius);
        float f6 = (float) (LIBMATTI_JU_Random_NextDouble(&random) * (double) M_PI * 2.0);
        LIBMATTI_JOML_Matrix3f m;
        LIBMATTI_JOML_Matrix3f_Identity(&m);
        LIBMATTI_JOML_Matrix3f_RotateTowards(&m, -star.x, -star.y, -star.z, 0.0f, 1.0f, 0.0f);
        LIBMATTI_JOML_Matrix3f_RotateZ(&m, -f6);
        const float corners[4][2] = {{f4, -f4}, {f4, f4}, {-f4, f4}, {-f4, -f4}};
        for (int c = 0; c < 4; c++)
        {
            float x = corners[c][0], y = corners[c][1], z = 0.0f;
            LIBMATTI_JOML_Matrix3f_Transform(&m, &x, &y, &z);
            vertices[vertexCount * 3 + 0] = x + star.x;
            vertices[vertexCount * 3 + 1] = y + star.y;
            vertices[vertexCount * 3 + 2] = z + star.z;
            vertexCount++;
        }
    }

    renderer->starIndexCount = vertexCount / 4 * 6;
    renderer->starBuffer.vertexCount = vertexCount;
    renderer->starBuffer.indexCount = renderer->starIndexCount;
    unsigned int vbos[1];
    LIBMATTI_GL_glGenBuffers(1, vbos);
    if (vbos[0] == 0)
    {
        free(vertices);
        return 0;
    }
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbos[0]);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, (long) vertexCount * POS_VERT_SIZE, vertices,
                             LIBMATTI_GL_GL_STATIC_DRAW);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    free(vertices);
    renderer->starBuffer.vbo = vbos[0];
    return 1;
}

// Java: private void buildSkyDisc(VertexConsumer, float) - the TRIANGLE_FAN
// around (0, y, 0) with the 9 rim points from -180 to 180 degrees; the port
// expands the fan into triangles so the draw is plain TRIANGLES.
static void fill_sky_disc(float *vertices, float y, int *outCount)
{
    float f = (y > 0.0f ? 1.0f : -1.0f) * 512.0f;
    float centre[3] = {0.0f, y, 0.0f};
    float rim[9][3];
    for (int i = -180, r = 0; i <= 180; i += 45, r++)
    {
        rim[r][0] = f * LIBMATTI_MC_Mth_Cos(i * (float) (M_PI / 180.0));
        rim[r][1] = y;
        rim[r][2] = 512.0f * LIBMATTI_MC_Mth_Sin(i * (float) (M_PI / 180.0));
    }
    int count = 0;
    for (int i = 0; i < 8; i++)
    {
        vertices[count * 3 + 0] = centre[0]; vertices[count * 3 + 1] = centre[1]; vertices[count * 3 + 2] = centre[2];
        count++;
        vertices[count * 3 + 0] = rim[i][0]; vertices[count * 3 + 1] = rim[i][1]; vertices[count * 3 + 2] = rim[i][2];
        count++;
        vertices[count * 3 + 0] = rim[i + 1][0]; vertices[count * 3 + 1] = rim[i + 1][1];
        vertices[count * 3 + 2] = rim[i + 1][2];
        count++;
    }
    *outCount = count;
}

static int build_sky_disc(SkyBuffer *buffer, float y)
{
    float vertices[24 * 3];
    int count = 0;
    fill_sky_disc(vertices, y, &count);
    unsigned int vbos[1];
    LIBMATTI_GL_glGenBuffers(1, vbos);
    if (vbos[0] == 0)
        return 0;
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbos[0]);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, (long) count * POS_VERT_SIZE, vertices,
                             LIBMATTI_GL_GL_STATIC_DRAW);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    buffer->vbo = vbos[0];
    buffer->vertexCount = count;
    buffer->indexCount = 0;
    return 1;
}

// Java: private GpuBuffer buildSunriseFan() - the fan around (0, 100, 0) over
// the 16-step circle, centre white, rim alpha 0; the port expands to triangles.
static int build_sunrise_fan(SkyBuffer *buffer)
{
    const int steps = 16;
    const float radius = 120.0f;
    float vertices[steps * 3 * 5]; // 16 triangles, 3 vertices, pos+color
    float ring[steps + 1][5];
    for (int i = 0; i <= steps; i++)
    {
        float f = (float) i * (float) (M_PI * 2.0) / 16.0f;
        ring[i][0] = LIBMATTI_MC_Mth_Sin(f) * radius;
        ring[i][1] = LIBMATTI_MC_Mth_Cos(f) * radius;
        ring[i][2] = -LIBMATTI_MC_Mth_Cos(f) * 40.0f;
        ring[i][3] = 1.0f;
        ring[i][4] = 0.0f; // Java: ARGB.white(0.0F) -> alpha 0
    }
    int count = 0;
    for (int i = 0; i < steps; i++)
    {
        vertices[count * 5 + 0] = 0.0f; vertices[count * 5 + 1] = 100.0f; vertices[count * 5 + 2] = 0.0f;
        vertices[count * 5 + 3] = 1.0f; vertices[count * 5 + 4] = 1.0f; // Java: ARGB.white(1.0F)
        count++;
        vertices[count * 5 + 0] = ring[i][0]; vertices[count * 5 + 1] = ring[i][1]; vertices[count * 5 + 2] = ring[i][2];
        vertices[count * 5 + 3] = ring[i][3]; vertices[count * 5 + 4] = ring[i][4];
        count++;
        vertices[count * 5 + 0] = ring[i + 1][0]; vertices[count * 5 + 1] = ring[i + 1][1];
        vertices[count * 5 + 2] = ring[i + 1][2];
        vertices[count * 5 + 3] = ring[i + 1][3]; vertices[count * 5 + 4] = ring[i + 1][4];
        count++;
    }
    unsigned int vbos[1];
    LIBMATTI_GL_glGenBuffers(1, vbos);
    if (vbos[0] == 0)
        return 0;
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbos[0]);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER, (long) count * POS_COLOR_VERT_SIZE, vertices,
                             LIBMATTI_GL_GL_STATIC_DRAW);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    buffer->vbo = vbos[0];
    buffer->vertexCount = count;
    buffer->indexCount = 0;
    return 1;
}

// The shared shader sources. The SKY pipeline draws the disc through the
// vertex color (Java: the SKY shader's vertex-color output); CELESTIAL +
// STARS sample/blend through the uniform tint.
static const char *SKY_VS =
    "#version 150\n"
    "uniform mat4 mvp;\n"
    "in vec3 Position;\n"
    "void main() { gl_Position = mvp * vec4(Position, 1.0); }\n";

static const char *SKY_FS =
    "#version 150\n"
    "uniform vec4 color;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = color; }\n";

static const char *CELESTIAL_VS =
    "#version 150\n"
    "uniform mat4 mvp;\n"
    "in vec3 Position;\n"
    "in vec2 UV;\n"
    "out vec2 texCoord;\n"
    "void main() { gl_Position = mvp * vec4(Position, 1.0); texCoord = UV; }\n";

static const char *CELESTIAL_FS =
    "#version 150\n"
    "uniform sampler2D Sampler0;\n"
    "uniform vec4 color;\n"
    "in vec2 texCoord;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = texture(Sampler0, texCoord) * color; }\n";

static const char *STARS_VS =
    "#version 150\n"
    "uniform mat4 mvp;\n"
    "in vec3 Position;\n"
    "void main() { gl_Position = mvp * vec4(Position, 1.0); }\n";

static const char *STARS_FS =
    "#version 150\n"
    "uniform vec4 color;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = color; }\n";

static const char *SUNRISE_VS =
    "#version 150\n"
    "uniform mat4 mvp;\n"
    "in vec3 Position;\n"
    "in vec4 Color;\n"
    "out vec4 vtxColor;\n"
    "void main() { gl_Position = mvp * vec4(Position, 1.0); vtxColor = Color; }\n";

static const char *SUNRISE_FS =
    "#version 150\n"
    "in vec4 vtxColor;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = vtxColor; }\n";

static unsigned int compile_shader(unsigned int type, const char *source)
{
    unsigned int shader = LIBMATTI_GL_glCreateShader(type);
    if (shader == 0)
        return 0;
    LIBMATTI_GL_glShaderSource(shader, source);
    LIBMATTI_GL_glCompileShader(shader);
    if (LIBMATTI_GL_glGetShaderi(shader, LIBMATTI_GL_GL_COMPILE_STATUS) == 0)
    {
        LIBMATTI_GL_glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static unsigned int link_program(const char *vsSource, const char *fsSource, const char *const *attribs, int attribCount)
{
    unsigned int vs = compile_shader(LIBMATTI_GL_GL_VERTEX_SHADER, vsSource);
    unsigned int fs = compile_shader(LIBMATTI_GL_GL_FRAGMENT_SHADER, fsSource);
    if (vs == 0 || fs == 0)
    {
        if (vs != 0) LIBMATTI_GL_glDeleteShader(vs);
        if (fs != 0) LIBMATTI_GL_glDeleteShader(fs);
        return 0;
    }
    unsigned int program = LIBMATTI_GL_glCreateProgram();
    LIBMATTI_GL_glAttachShader(program, vs);
    LIBMATTI_GL_glAttachShader(program, fs);
    for (int i = 0; i < attribCount; i++)
        LIBMATTI_GL_glBindAttribLocation(program, (unsigned int) i, attribs[i]);
    LIBMATTI_GL_glLinkProgram(program);
    LIBMATTI_GL_glDeleteShader(vs);
    LIBMATTI_GL_glDeleteShader(fs);
    if (LIBMATTI_GL_glGetProgrami(program, LIBMATTI_GL_GL_LINK_STATUS) == 0)
    {
        LIBMATTI_GL_glDeleteProgram(program);
        return 0;
    }
    return program;
}

// The per-pass VAO over the pass's vertex layout.
static unsigned int create_vao(unsigned int vbo, int positionOnly)
{
    unsigned int vaos[1];
    LIBMATTI_GL_glGenVertexArrays(1, vaos);
    if (vaos[0] == 0)
        return 0;
    LIBMATTI_GL_glBindVertexArray(vaos[0]);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbo);
    if (positionOnly)
    {
        LIBMATTI_GL_glEnableVertexAttribArray(0);
        LIBMATTI_GL_glVertexAttribPointer(0, 3, LIBMATTI_GL_GL_FLOAT, 0, 12, NULL);
    }
    else
    {
        // POSITION_COLOR: pos 3f + color 4f.
        LIBMATTI_GL_glEnableVertexAttribArray(0);
        LIBMATTI_GL_glVertexAttribPointer(0, 3, LIBMATTI_GL_GL_FLOAT, 0, 20, NULL);
        LIBMATTI_GL_glEnableVertexAttribArray(1);
        LIBMATTI_GL_glVertexAttribPointer(1, 4, LIBMATTI_GL_GL_FLOAT, 0, 20, (const void *) 12);
    }
    LIBMATTI_GL_glBindVertexArray(0);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    return vaos[0];
}

static unsigned int create_celestial_vao(unsigned int vbo)
{
    unsigned int vaos[1];
    LIBMATTI_GL_glGenVertexArrays(1, vaos);
    if (vaos[0] == 0)
        return 0;
    LIBMATTI_GL_glBindVertexArray(vaos[0]);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbo);
    // POSITION_TEX: pos 3f + uv 2f.
    LIBMATTI_GL_glEnableVertexAttribArray(0);
    LIBMATTI_GL_glVertexAttribPointer(0, 3, LIBMATTI_GL_GL_FLOAT, 0, 20, NULL);
    LIBMATTI_GL_glEnableVertexAttribArray(1);
    LIBMATTI_GL_glVertexAttribPointer(1, 2, LIBMATTI_GL_GL_FLOAT, 0, 20, (const void *) 12);
    LIBMATTI_GL_glBindVertexArray(0);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    return vaos[0];
}

// Java: RenderSystem.getSequentialBuffer(QUADS).getBuffer(n) - the shared quad
// element indices as UNSIGNED_SHORT.
static unsigned int ensure_quad_ibo(LIBMATTI_MC_SkyRenderer *renderer, int quadCount)
{
    if (renderer->quadIbo != 0 && quadCount <= renderer->quadIboCapacity)
        return renderer->quadIbo;
    if (renderer->quadIbo == 0)
    {
        unsigned int ibos[1];
        LIBMATTI_GL_glGenBuffers(1, ibos);
        renderer->quadIbo = ibos[0];
    }
    if (renderer->quadIbo == 0)
        return 0;
    unsigned short *indices = malloc((size_t) quadCount * 6 * sizeof(unsigned short));
    if (indices == NULL)
        return 0;
    for (int q = 0; q < quadCount; q++)
    {
        unsigned short base = (unsigned short) (q * 4);
        unsigned short *target = indices + q * 6;
        target[0] = base;
        target[1] = base + 1;
        target[2] = base + 2;
        target[3] = base + 2;
        target[4] = base + 1;
        target[5] = base + 3;
    }
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, renderer->quadIbo);
    LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, (long) quadCount * 6 * 2, indices,
                             LIBMATTI_GL_GL_DYNAMIC_DRAW);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
    free(indices);
    renderer->quadIboCapacity = quadCount;
    return renderer->quadIbo;
}

// Java: SkyRenderer(TextureManager, AtlasManager) - the buffers build once in
// the constructor.
LIBMATTI_MC_SkyRenderer *LIBMATTI_MC_SkyRenderer_New(void)
{
    LIBMATTI_MC_SkyRenderer *renderer = calloc(1, sizeof(LIBMATTI_MC_SkyRenderer));
    if (renderer == NULL)
        return NULL;

    renderer->celestialsAtlas = NULL;
    renderer->skyProgram = 0;
    renderer->celestialProgram = 0;
    renderer->starsProgram = 0;
    renderer->sunriseProgram = 0;

    // The top/bottom sky discs build even without a GL context (the VBO
    // uploads degrade to the binding's no-context fallbacks, like the
    // section path).
    build_sky_disc(&renderer->topSkyBuffer, 16.0f);
    build_sky_disc(&renderer->bottomSkyBuffer, -16.0f);
    return renderer;
}

// Java: the celestials atlas rides in through the Minecraft bootstrap (the
// CELESTIALS atlas load); the celestial quads build against it.
void LIBMATTI_MC_SkyRenderer_SetAtlas(LIBMATTI_MC_SkyRenderer *renderer, void *celestialsAtlas)
{
    if (renderer == NULL)
        return;
    renderer->celestialsAtlas = celestialsAtlas;
    if (renderer->celestialsAtlas != NULL)
    {
        build_sun_quad(&renderer->sunBuffer, (const LIBMATTI_MC_TextureAtlas *) renderer->celestialsAtlas);
        build_moon_phases(&renderer->moonBuffer, (const LIBMATTI_MC_TextureAtlas *) renderer->celestialsAtlas);
    }
    build_stars(renderer);
    build_sunrise_fan(&renderer->sunriseBuffer);
}

void LIBMATTI_MC_SkyRenderer_Free(LIBMATTI_MC_SkyRenderer *renderer)
{
    if (renderer == NULL)
        return;
    unsigned int vbos[6];
    int count = 0;
    if (renderer->starBuffer.vbo != 0) vbos[count++] = renderer->starBuffer.vbo;
    if (renderer->topSkyBuffer.vbo != 0) vbos[count++] = renderer->topSkyBuffer.vbo;
    if (renderer->bottomSkyBuffer.vbo != 0) vbos[count++] = renderer->bottomSkyBuffer.vbo;
    if (renderer->sunBuffer.vbo != 0) vbos[count++] = renderer->sunBuffer.vbo;
    if (renderer->moonBuffer.vbo != 0) vbos[count++] = renderer->moonBuffer.vbo;
    if (renderer->sunriseBuffer.vbo != 0) vbos[count++] = renderer->sunriseBuffer.vbo;
    if (count > 0)
        LIBMATTI_GL_glDeleteBuffers(count, vbos);
    if (renderer->quadIbo != 0)
        LIBMATTI_GL_glDeleteBuffers(1, &renderer->quadIbo);
    if (renderer->skyProgram != 0) LIBMATTI_GL_glDeleteProgram(renderer->skyProgram);
    if (renderer->celestialProgram != 0) LIBMATTI_GL_glDeleteProgram(renderer->celestialProgram);
    if (renderer->starsProgram != 0) LIBMATTI_GL_glDeleteProgram(renderer->starsProgram);
    if (renderer->sunriseProgram != 0) LIBMATTI_GL_glDeleteProgram(renderer->sunriseProgram);
    free(renderer);
}

// The lazy program + VAO pairing per pass kind. The VAOs ride the SkyBuffer's
// vbo handle; the port caches them in small statics keyed by the handle.
static unsigned int ensure_sky_program(LIBMATTI_MC_SkyRenderer *renderer)
{
    if (renderer->skyProgram == 0)
    {
        const char *attribs[1] = {"Position"};
        renderer->skyProgram = link_program(SKY_VS, SKY_FS, attribs, 1);
    }
    return renderer->skyProgram;
}

static unsigned int ensure_celestial_program(LIBMATTI_MC_SkyRenderer *renderer)
{
    if (renderer->celestialProgram == 0)
    {
        const char *attribs[2] = {"Position", "UV"};
        renderer->celestialProgram = link_program(CELESTIAL_VS, CELESTIAL_FS, attribs, 2);
    }
    return renderer->celestialProgram;
}

static unsigned int ensure_stars_program(LIBMATTI_MC_SkyRenderer *renderer)
{
    if (renderer->starsProgram == 0)
    {
        const char *attribs[1] = {"Position"};
        renderer->starsProgram = link_program(STARS_VS, STARS_FS, attribs, 1);
    }
    return renderer->starsProgram;
}

static unsigned int ensure_sunrise_program(LIBMATTI_MC_SkyRenderer *renderer)
{
    if (renderer->sunriseProgram == 0)
    {
        const char *attribs[2] = {"Position", "Color"};
        renderer->sunriseProgram = link_program(SUNRISE_VS, SUNRISE_FS, attribs, 2);
    }
    return renderer->sunriseProgram;
}

// Java: the dynamic transform upload - the port sets the mvp uniform (the
// model-view matrix multiplied onto the caller's projection) and the tint.
static void set_transform(unsigned int program, const LIBMATTI_JOML_Matrix4f *modelView,
                          const LIBMATTI_JOML_Matrix4f *projection, float r, float g, float b, float a)
{
    LIBMATTI_JOML_Matrix4f mvp;
    LIBMATTI_JOML_Matrix4f_Mul(projection, modelView, &mvp);
    int mvpLocation = LIBMATTI_GL_glGetUniformLocation(program, "mvp");
    if (mvpLocation >= 0)
        LIBMATTI_GL_glUniformMatrix4fv(mvpLocation, 0, (const float *) &mvp);
    int colorLocation = LIBMATTI_GL_glGetUniformLocation(program, "color");
    if (colorLocation >= 0)
        LIBMATTI_GL_glUniform4f(colorLocation, r, g, b, a);
}

// The draw helpers keep the pass structure of the Java class: each renders one
// buffer through its pipeline with the model-view stack multiplied on.
void LIBMATTI_MC_SkyRenderer_DrawSkyDisc(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                         const LIBMATTI_JOML_Matrix4f *projection, float r, float g, float b, float a)
{
    unsigned int program = ensure_sky_program(renderer);
    if (program == 0)
        return;
    LIBMATTI_GL_glUseProgram(program);
    set_transform(program, modelView, projection, r, g, b, a);
    static unsigned int topVao = 0, bottomVao = 0;
    static unsigned int boundTopVbo = 0, boundBottomVbo = 0;
    if (topVao == 0 || boundTopVbo != renderer->topSkyBuffer.vbo)
    {
        if (topVao != 0) LIBMATTI_GL_glDeleteVertexArrays(1, &topVao);
        topVao = create_vao(renderer->topSkyBuffer.vbo, 1);
        boundTopVbo = renderer->topSkyBuffer.vbo;
    }
    LIBMATTI_GL_glBindVertexArray(topVao);
    LIBMATTI_GL_glDrawArrays(LIBMATTI_GL_GL_TRIANGLES, 0, renderer->topSkyBuffer.vertexCount);
    LIBMATTI_GL_glBindVertexArray(0);
}

// Java: public void renderDarkDisc() - the bottom disc, translated +12 on y.
void LIBMATTI_MC_SkyRenderer_DrawDarkDisc(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                          const LIBMATTI_JOML_Matrix4f *projection)
{
    unsigned int program = ensure_sky_program(renderer);
    if (program == 0)
        return;
    LIBMATTI_JOML_Matrix4f translated;
    memcpy(&translated, modelView, sizeof(translated));
    LIBMATTI_JOML_Matrix4f_Translate(&translated, 0.0f, 12.0f, 0.0f);
    LIBMATTI_GL_glUseProgram(program);
    set_transform(program, &translated, projection, 0.0f, 0.0f, 0.0f, 1.0f);
    static unsigned int bottomVao = 0, boundBottomVbo = 0;
    if (bottomVao == 0 || boundBottomVbo != renderer->bottomSkyBuffer.vbo)
    {
        if (bottomVao != 0) LIBMATTI_GL_glDeleteVertexArrays(1, &bottomVao);
        bottomVao = create_vao(renderer->bottomSkyBuffer.vbo, 1);
        boundBottomVbo = renderer->bottomSkyBuffer.vbo;
    }
    LIBMATTI_GL_glBindVertexArray(bottomVao);
    LIBMATTI_GL_glDrawArrays(LIBMATTI_GL_GL_TRIANGLES, 0, renderer->bottomSkyBuffer.vertexCount);
    LIBMATTI_GL_glBindVertexArray(0);
}

// Java: private void renderSun(float, PoseStack) - the sun quad at
// (0, 100, 0) scaled (30, 1, 30), tint alpha = the rain brightness.
void LIBMATTI_MC_SkyRenderer_DrawSun(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                     const LIBMATTI_JOML_Matrix4f *projection, float alpha)
{
    unsigned int program = ensure_celestial_program(renderer);
    if (program == 0 || renderer->sunBuffer.vbo == 0)
        return;
    LIBMATTI_JOML_Matrix4f m;
    memcpy(&m, modelView, sizeof(m));
    LIBMATTI_JOML_Matrix4f_Translate(&m, 0.0f, 100.0f, 0.0f);
    LIBMATTI_JOML_Matrix4f_Scale(&m, 30.0f, 1.0f, 30.0f);
    LIBMATTI_GL_glUseProgram(program);
    set_transform(program, &m, projection, 1.0f, 1.0f, 1.0f, alpha);
    LIBMATTI_MC_AbstractTexture *texture = &((LIBMATTI_MC_TextureAtlas *) renderer->celestialsAtlas)->base;
    LIBMATTI_B3D_GlStateManager_BindTexture(texture->texture);
    static unsigned int sunVao = 0, boundSunVbo = 0;
    if (sunVao == 0 || boundSunVbo != renderer->sunBuffer.vbo)
    {
        if (sunVao != 0) LIBMATTI_GL_glDeleteVertexArrays(1, &sunVao);
        sunVao = create_celestial_vao(renderer->sunBuffer.vbo);
        boundSunVbo = renderer->sunBuffer.vbo;
    }
    unsigned int ibo = ensure_quad_ibo(renderer, 1);
    if (ibo == 0)
        return;
    LIBMATTI_GL_glBindVertexArray(sunVao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, ibo);
    LIBMATTI_GL_glDrawElements(LIBMATTI_GL_GL_TRIANGLES, 6, LIBMATTI_GL_GL_UNSIGNED_SHORT, NULL);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
    LIBMATTI_GL_glBindVertexArray(0);
}

// Java: private void renderMoon(MoonPhase, float, PoseStack) - the phase quad
// at (0, 100, 0) scaled (20, 1, 20); the draw indexes at phase * 4.
void LIBMATTI_MC_SkyRenderer_DrawMoon(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                      const LIBMATTI_JOML_Matrix4f *projection, LIBMATTI_MC_MoonPhase phase,
                                      float alpha)
{
    unsigned int program = ensure_celestial_program(renderer);
    if (program == 0 || renderer->moonBuffer.vbo == 0)
        return;
    LIBMATTI_JOML_Matrix4f m;
    memcpy(&m, modelView, sizeof(m));
    LIBMATTI_JOML_Matrix4f_Translate(&m, 0.0f, 100.0f, 0.0f);
    LIBMATTI_JOML_Matrix4f_Scale(&m, 20.0f, 1.0f, 20.0f);
    LIBMATTI_GL_glUseProgram(program);
    set_transform(program, &m, projection, 1.0f, 1.0f, 1.0f, alpha);
    LIBMATTI_MC_AbstractTexture *texture = &((LIBMATTI_MC_TextureAtlas *) renderer->celestialsAtlas)->base;
    LIBMATTI_B3D_GlStateManager_BindTexture(texture->texture);
    static unsigned int moonVao = 0, boundMoonVbo = 0;
    if (moonVao == 0 || boundMoonVbo != renderer->moonBuffer.vbo)
    {
        if (moonVao != 0) LIBMATTI_GL_glDeleteVertexArrays(1, &moonVao);
        moonVao = create_celestial_vao(renderer->moonBuffer.vbo);
        boundMoonVbo = renderer->moonBuffer.vbo;
    }
    unsigned int ibo = ensure_quad_ibo(renderer, LIBMATTI_MC_MoonPhase_COUNT);
    if (ibo == 0)
        return;
    LIBMATTI_GL_glBindVertexArray(moonVao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, ibo);
    // Java: drawIndexed(i, 0, 6, 1) with i = phase.index() * 4 (in elements: * 6).
    int firstIndex = LIBMATTI_MC_MoonPhase_Index(phase) * 6;
    LIBMATTI_GL_glDrawElementsBaseVertex(LIBMATTI_GL_GL_TRIANGLES, 6, LIBMATTI_GL_GL_UNSIGNED_SHORT,
                                         (const void *) (long) (firstIndex * sizeof(unsigned short)), firstIndex / 6 * 4);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
    LIBMATTI_GL_glBindVertexArray(0);
}

// Java: private void renderStars(float, PoseStack) - the star quads tinted by
// the brightness scalar through the color uniform.
void LIBMATTI_MC_SkyRenderer_DrawStars(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                       const LIBMATTI_JOML_Matrix4f *projection, float brightness)
{
    unsigned int program = ensure_stars_program(renderer);
    if (program == 0 || renderer->starBuffer.vbo == 0 || brightness <= 0.0f)
        return;
    LIBMATTI_GL_glUseProgram(program);
    set_transform(program, modelView, projection, brightness, brightness, brightness, brightness);
    static unsigned int starVao = 0, boundStarVbo = 0;
    if (starVao == 0 || boundStarVbo != renderer->starBuffer.vbo)
    {
        if (starVao != 0) LIBMATTI_GL_glDeleteVertexArrays(1, &starVao);
        starVao = create_vao(renderer->starBuffer.vbo, 1);
        boundStarVbo = renderer->starBuffer.vbo;
    }
    unsigned int ibo = ensure_quad_ibo(renderer, renderer->starIndexCount / 6);
    if (ibo == 0)
        return;
    LIBMATTI_GL_glBindVertexArray(starVao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, ibo);
    LIBMATTI_GL_glDrawElements(LIBMATTI_GL_GL_TRIANGLES, renderer->starIndexCount, LIBMATTI_GL_GL_UNSIGNED_SHORT, NULL);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
    LIBMATTI_GL_glBindVertexArray(0);
}

// Java: public void renderSunriseAndSunset(PoseStack, float, int) - the fan
// scaled by (1, 1, alpha) around the X rotation; the color rides the vertex
// colors, the alpha through the model-view's z scale like Java's writeTransform.
void LIBMATTI_MC_SkyRenderer_DrawSunriseSunset(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                               const LIBMATTI_JOML_Matrix4f *projection, float angleRadians, float alpha)
{
    unsigned int program = ensure_sunrise_program(renderer);
    if (program == 0 || renderer->sunriseBuffer.vbo == 0 || alpha <= 0.001f)
        return;
    LIBMATTI_JOML_Matrix4f m;
    memcpy(&m, modelView, sizeof(m));
    // Java: mulPose(Axis.XP.rotationDegrees(90)) then the Z flip by the
    // sunrise/sunset side, then scale(1, 1, alpha).
    LIBMATTI_JOML_Matrix4f_Rotate(&m, (float) (M_PI / 2.0), 1.0f, 0.0f, 0.0f);
    float flip = LIBMATTI_MC_Mth_Sin(angleRadians) < 0.0f ? 180.0f : 0.0f;
    LIBMATTI_JOML_Matrix4f_Rotate(&m, (float) ((flip + 90.0f) * (M_PI / 180.0)), 0.0f, 0.0f, 1.0f);
    LIBMATTI_JOML_Matrix4f_Scale(&m, 1.0f, 1.0f, alpha);
    LIBMATTI_GL_glUseProgram(program);
    set_transform(program, &m, projection, 1.0f, 1.0f, 1.0f, 1.0f);
    static unsigned int sunriseVao = 0, boundSunriseVbo = 0;
    if (sunriseVao == 0 || boundSunriseVbo != renderer->sunriseBuffer.vbo)
    {
        if (sunriseVao != 0) LIBMATTI_GL_glDeleteVertexArrays(1, &sunriseVao);
        sunriseVao = create_vao(renderer->sunriseBuffer.vbo, 0);
        boundSunriseVbo = renderer->sunriseBuffer.vbo;
    }
    LIBMATTI_GL_glBindVertexArray(sunriseVao);
    LIBMATTI_GL_glDrawArrays(LIBMATTI_GL_GL_TRIANGLES, 0, renderer->sunriseBuffer.vertexCount);
    LIBMATTI_GL_glBindVertexArray(0);
}

// Java: public void renderSunMoonAndStars(PoseStack, sunAngle, moonAngle,
// starAngle, moonPhase, rainBrightness, starBrightness) - the Y -90 spin and
// the per-body X rotations around the PoseStack.
void LIBMATTI_MC_SkyRenderer_RenderSunMoonAndStars(LIBMATTI_MC_SkyRenderer *renderer,
                                                   const LIBMATTI_JOML_Matrix4f *modelView,
                                                   const LIBMATTI_JOML_Matrix4f *projection, float sunAngle,
                                                   float moonAngle, float starAngle, LIBMATTI_MC_MoonPhase moonPhase,
                                                   float rainBrightness, float starBrightness)
{
    LIBMATTI_JOML_Matrix4f base;
    memcpy(&base, modelView, sizeof(base));
    // Java: mulPose(Axis.YP.rotationDegrees(-90.0F))
    LIBMATTI_JOML_Matrix4f_Rotate(&base, (float) (-90.0 * (M_PI / 180.0)), 0.0f, 1.0f, 0.0f);

    LIBMATTI_JOML_Matrix4f sunPose;
    memcpy(&sunPose, &base, sizeof(sunPose));
    LIBMATTI_JOML_Matrix4f_Rotate(&sunPose, sunAngle, 1.0f, 0.0f, 0.0f);
    LIBMATTI_MC_SkyRenderer_DrawSun(renderer, &sunPose, projection, rainBrightness);

    LIBMATTI_JOML_Matrix4f moonPose;
    memcpy(&moonPose, &base, sizeof(moonPose));
    LIBMATTI_JOML_Matrix4f_Rotate(&moonPose, moonAngle, 1.0f, 0.0f, 0.0f);
    LIBMATTI_MC_SkyRenderer_DrawMoon(renderer, &moonPose, projection, moonPhase, rainBrightness);

    if (starBrightness > 0.0f)
    {
        LIBMATTI_JOML_Matrix4f starPose;
        memcpy(&starPose, &base, sizeof(starPose));
        LIBMATTI_JOML_Matrix4f_Rotate(&starPose, starAngle, 1.0f, 0.0f, 0.0f);
        LIBMATTI_MC_SkyRenderer_DrawStars(renderer, &starPose, projection, starBrightness);
    }
}

// Port of net.minecraft.client.renderer.CloudRenderer (implementation).
//
// The mesh builder mirrors the Java lines: the diamond iteration over the
// camera cell ring (buildMesh), the neighbour-flag face culling for the
// extruded FANCY cells (buildExtrudedCell), the flat FAST cell and the
// encodeFace cell offsets. Java uploads packed cell bytes and expands them
// in the vertex shader; the port expands at build time into position+uv
// triangles - the resulting geometry is the same.

#include "libmatti/net/minecraft/client/renderer/CloudRenderer.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/net/minecraft/util/Mth.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final int FLAG_INSIDE_FACE = 16, FLAG_USE_TOP_COLOR = 32
#define FLAG_INSIDE_FACE 16
#define FLAG_USE_TOP_COLOR 32
// Java: the neighbour bits NORTH/EAST/SOUTH/WEST = 3/2/1/0
#define NORTH_EMPTY_BIT 3
#define EAST_EMPTY_BIT 2
#define SOUTH_EMPTY_BIT 1
#define WEST_EMPTY_BIT 0

// Java: DefaultVertexFormat.POSITION_TEX's vertex size (pos 3f + uv 2f).
#define CLOUD_VERT_SIZE 20

// Java: record TextureData(long[] cells, int width, int height)
typedef struct TextureData
{
    unsigned long *cells;
    int width;
    int height;
} TextureData;

struct LIBMATTI_MC_CloudRenderer
{
    TextureData texture;
    // Java: private boolean needsRebuild
    int needsRebuild;
    // Java: private int prevCellX / prevCellZ
    int prevCellX;
    int prevCellZ;
    LIBMATTI_MC_Cloud_RelativeCameraPos prevRelativeCameraPos;
    LIBMATTI_MC_CloudStatus prevType;
    // The expanded triangle mesh (Java: the UTB ring buffer's packed cells;
    // the port keeps expanded position+uv triangles and uploads on rebuild).
    float *meshVertices;
    int meshVertexCount;
    int meshCapacity; // in vertices
    unsigned int meshVbo;
    unsigned int cloudProgram;
    unsigned int cloudTexture;
    // Java: private int quadCount (the port counts triangles)
    int triangleCount;
};

// ---------------------------------------------------------------------------
// The clouds texture (256x256): Java reads textures/environment/clouds.png;
// the vendor repo carries no binary assets, so the port generates the same
// kind of wrapped blob field.
// ---------------------------------------------------------------------------

static int is_cell_empty(unsigned int argb)
{
    // Java: ARGB.alpha(p) < 10
    return (int) ((argb >> 24) & 0xFFu) < 10;
}

static unsigned long pack_cell_data(unsigned int color, int northEmpty, int eastEmpty,
                                    int southEmpty, int westEmpty)
{
    // Java: (long) color << 4 | north << 3 | east << 2 | south << 1 | west
    return ((unsigned long) color << 4)
         | ((unsigned long) (northEmpty ? 1 : 0) << NORTH_EMPTY_BIT)
         | ((unsigned long) (eastEmpty ? 1 : 0) << EAST_EMPTY_BIT)
         | ((unsigned long) (southEmpty ? 1 : 0) << SOUTH_EMPTY_BIT)
         | ((unsigned long) (westEmpty ? 1 : 0) << WEST_EMPTY_BIT);
}

static int is_north_empty(unsigned long cell) { return (int) ((cell >> NORTH_EMPTY_BIT) & 1UL); }
static int is_east_empty(unsigned long cell) { return (int) ((cell >> EAST_EMPTY_BIT) & 1UL); }
static int is_south_empty(unsigned long cell) { return (int) ((cell >> SOUTH_EMPTY_BIT) & 1UL); }
static int is_west_empty(unsigned long cell) { return (int) ((cell >> WEST_EMPTY_BIT) & 1UL); }

// The blob field: a seeded scatter of rounded blobs, wrapped on the 256x256
// torus (the vanilla texture tiles the same way).
static unsigned int *generate_cloud_image(int *outW, int *outH)
{
    const int size = 256;
    unsigned int *image = calloc((size_t) size * size, sizeof(unsigned int));
    if (image == NULL)
        return NULL;
    unsigned int seed = 10842u;
    for (int blob = 0; blob < 48; blob++)
    {
        seed = seed * 1664525u + 1013904223u;
        int cx = (int) ((seed >> 16) % (unsigned) size);
        seed = seed * 1664525u + 1013904223u;
        int cy = (int) ((seed >> 16) % (unsigned) size);
        seed = seed * 1664525u + 1013904223u;
        int rx = 6 + (int) ((seed >> 16) % 14u);
        seed = seed * 1664525u + 1013904223u;
        int ry = 5 + (int) ((seed >> 16) % 10u);
        for (int dy = -ry; dy <= ry; dy++)
        {
            for (int dx = -rx; dx <= rx; dx++)
            {
                float nx = (float) dx / (float) rx;
                float ny = (float) dy / (float) ry;
                if (nx * nx + ny * ny <= 1.0f)
                {
                    int x = (cx + dx + size) % size;
                    int y = (cy + dy + size) % size;
                    image[x + y * size] = 0xFFFFFFFFu; // white, alpha 255
                }
            }
        }
    }
    *outW = size;
    *outH = size;
    return image;
}

// Java: prepare(ResourceManager, ProfilerFiller) - the cell packing loop
// over the image with the wrapped neighbour lookups.
static int prepare_texture(TextureData *data)
{
    int width = 0, height = 0;
    unsigned int *image = generate_cloud_image(&width, &height);
    if (image == NULL)
        return 0;
    data->width = width;
    data->height = height;
    data->cells = malloc(sizeof(unsigned long) * (size_t) width * height);
    if (data->cells == NULL)
    {
        free(image);
        return 0;
    }
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned int pixel = image[x + y * width];
            if (is_cell_empty(pixel))
            {
                data->cells[x + y * width] = 0UL;
                continue;
            }
            int northEmpty = is_cell_empty(image[x + ((y - 1 + height) % height) * width]);
            int eastEmpty = is_cell_empty(image[((x + 1) % width) + y * width]);
            int southEmpty = is_cell_empty(image[x + ((y + 1) % height) * width]);
            int westEmpty = is_cell_empty(image[((x - 1 + width) % width) + y * width]);
            data->cells[x + y * width] = pack_cell_data(pixel, northEmpty, eastEmpty, southEmpty, westEmpty);
        }
    }
    free(image);
    return 1;
}

// ---------------------------------------------------------------------------
// The mesh builder - 1:1 with Java's buildMesh / tryBuildCell /
// buildFlatCell / buildExtrudedCell / encodeFace. The mesh cells carry the
// RELATIVE ring offsets (l, i1) only, exactly like Java's packed encoding;
// the shader-origin supplies the base (see Render).
// ---------------------------------------------------------------------------

// The quad corners per face direction in the cell's local block space, wound
// so the two triangles (0,1,2)(2,1,3) face outward. Direction order is
// Direction.get3DDataValue(): DOWN 0, UP 1, NORTH 2, SOUTH 3, WEST 4, EAST 5.
typedef struct FaceQuad
{
    float corners[4][3];
} FaceQuad;

static const FaceQuad face_quads[6] = {
    // DOWN (y = 0)
    {{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 12.0f}, {12.0f, 0.0f, 12.0f}, {12.0f, 0.0f, 0.0f}}},
    // UP (y = 4)
    {{{0.0f, 4.0f, 0.0f}, {12.0f, 4.0f, 0.0f}, {12.0f, 4.0f, 12.0f}, {0.0f, 4.0f, 12.0f}}},
    // NORTH (z = 0)
    {{{12.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 4.0f, 0.0f}, {12.0f, 4.0f, 0.0f}}},
    // SOUTH (z = 12)
    {{{0.0f, 0.0f, 12.0f}, {12.0f, 0.0f, 12.0f}, {12.0f, 4.0f, 12.0f}, {0.0f, 4.0f, 12.0f}}},
    // WEST (x = 0)
    {{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 12.0f}, {0.0f, 4.0f, 12.0f}, {0.0f, 4.0f, 0.0f}}},
    // EAST (x = 12)
    {{{12.0f, 0.0f, 12.0f}, {12.0f, 0.0f, 0.0f}, {12.0f, 4.0f, 0.0f}, {12.0f, 4.0f, 12.0f}}},
};

// Java: encodeFace appends the packed cell bytes; the port appends the
// expanded quad as two triangles (the sequential quad buffer's expansion).
static void append_face(LIBMATTI_MC_CloudRenderer *renderer, int relCellX, int relCellZ,
                        int direction, int flags)
{
    (void) flags;
    if (renderer->meshVertexCount + 6 > renderer->meshCapacity)
    {
        int newCapacity = renderer->meshCapacity == 0 ? 4096 : renderer->meshCapacity * 2;
        float *grown = realloc(renderer->meshVertices, sizeof(float) * 5 * (size_t) newCapacity);
        if (grown == NULL)
            return;
        renderer->meshVertices = grown;
        renderer->meshCapacity = newCapacity;
    }
    const FaceQuad *quad = &face_quads[direction];
    // The cell origin from the RELATIVE ring offset (Java: (cell >> 1) * 12
    // reconstructed in the shader on top of the CloudInfo origin).
    float originX = (float) (relCellX * 12);
    float originZ = (float) (relCellZ * 12);
    // The uv: the texture repeats per cell (the cloud texture tiles over the
    // cell grid; the sampled footprint is one 12x4 face slice).
    const float uvs[4][2] = {{0.0f, 0.0f}, {0.75f, 0.0f}, {0.75f, 0.25f}, {0.0f, 0.25f}};
    static const int triangleOrder[6] = {0, 1, 2, 2, 1, 3};
    float *base = renderer->meshVertices + renderer->meshVertexCount * 5;
    for (int t = 0; t < 6; t++)
    {
        int i = triangleOrder[t];
        base[t * 5 + 0] = originX + quad->corners[i][0];
        base[t * 5 + 1] = quad->corners[i][1];
        base[t * 5 + 2] = originZ + quad->corners[i][2];
        base[t * 5 + 3] = uvs[i][0];
        base[t * 5 + 4] = uvs[i][1];
    }
    renderer->meshVertexCount += 6;
}

// Java: private void buildFlatCell(ByteBuffer, int, int) - the top face only
// (a DOWN-direction quad at y = 4 with the top color flag).
static void build_flat_cell(LIBMATTI_MC_CloudRenderer *renderer, int relCellX, int relCellZ)
{
    append_face(renderer, relCellX, relCellZ, 1, FLAG_USE_TOP_COLOR);
}

// Java: private void buildExtrudedCell(RelativeCameraPos, ByteBuffer, int, int, long)
static void build_extruded_cell(LIBMATTI_MC_CloudRenderer *renderer,
                                LIBMATTI_MC_Cloud_RelativeCameraPos relativePos,
                                int relCellX, int relCellZ, unsigned long cell)
{
    if (relativePos != LIBMATTI_MC_Cloud_BELOW_CLOUDS)
        append_face(renderer, relCellX, relCellZ, 1, 0); // UP
    if (relativePos != LIBMATTI_MC_Cloud_ABOVE_CLOUDS)
        append_face(renderer, relCellX, relCellZ, 0, 0); // DOWN
    if (is_north_empty(cell) && relCellZ > 0)
        append_face(renderer, relCellX, relCellZ, 2, 0); // NORTH
    if (is_south_empty(cell) && relCellZ < 0)
        append_face(renderer, relCellX, relCellZ, 3, 0); // SOUTH
    if (is_west_empty(cell) && relCellX > 0)
        append_face(renderer, relCellX, relCellZ, 4, 0); // WEST
    if (is_east_empty(cell) && relCellX < 0)
        append_face(renderer, relCellX, relCellZ, 5, 0); // EAST
    // Java: boolean flag = abs(x) <= 1 && abs(z) <= 1 - the inside faces.
    if (relCellX >= -1 && relCellX <= 1 && relCellZ >= -1 && relCellZ <= 1)
    {
        for (int direction = 0; direction < 6; direction++)
            append_face(renderer, relCellX, relCellZ, direction, FLAG_INSIDE_FACE);
    }
}

// Java: private void tryBuildCell(..., int baseX, int baseZ, boolean fancy,
// int relX, int width, int relZ, int height, long[] cells) - the texture
// lookup at floorMod(base + rel), the mesh at the RELATIVE offsets.
static void try_build_cell(LIBMATTI_MC_CloudRenderer *renderer,
                           LIBMATTI_MC_Cloud_RelativeCameraPos relativePos,
                           int baseCellX, int baseCellZ, int fancy,
                           int relX, int relZ)
{
    int tx = (baseCellX + relX) % renderer->texture.width;
    if (tx < 0)
        tx += renderer->texture.width;
    int ty = (baseCellZ + relZ) % renderer->texture.height;
    if (ty < 0)
        ty += renderer->texture.height;
    unsigned long cell = renderer->texture.cells[tx + ty * renderer->texture.width];
    if (cell == 0UL)
        return;
    if (fancy)
        build_extruded_cell(renderer, relativePos, relX, relZ, cell);
    else
        build_flat_cell(renderer, relX, relZ);
}

// Java: private void buildMesh(RelativeCameraPos, ByteBuffer, int cellX,
// int cellZ, boolean fancy, int cells) - the diamond ring iteration.
static void build_mesh(LIBMATTI_MC_CloudRenderer *renderer,
                       LIBMATTI_MC_Cloud_RelativeCameraPos relativePos,
                       int cellX, int cellZ, int fancy, int cells)
{
    renderer->meshVertexCount = 0;
    for (int k = 0; k <= 2 * cells; k++)
    {
        for (int l = -k; l <= k; l++)
        {
            int i1 = k - (l < 0 ? -l : l);
            if (i1 >= 0 && i1 <= cells && l * l + i1 * i1 <= cells * cells)
            {
                if (i1 != 0)
                    try_build_cell(renderer, relativePos, cellX, cellZ, fancy, l, -i1);
                try_build_cell(renderer, relativePos, cellX, cellZ, fancy, l, i1);
            }
        }
    }
    renderer->triangleCount = renderer->meshVertexCount / 3;
}

// ---------------------------------------------------------------------------
// The cloud program (Java: RenderPipelines.CLOUDS / FLAT_CLOUDS' shaders).
// The world anchor rides the origin uniform (Java: CloudInfo's vec3(-f3, f,
// -f4) on top of the DynamicTransforms model-view); the port folds the
// camera into it.
// ---------------------------------------------------------------------------

static const char *CLOUD_VS =
    "#version 150\n"
    "in vec3 Position;\n"
    "in vec2 UV0;\n"
    "uniform mat4 mvp;\n"
    "uniform vec3 origin;\n"
    "out vec2 texCoord;\n"
    "void main() {\n"
    "    gl_Position = mvp * vec4(Position + origin, 1.0);\n"
    "    texCoord = UV0;\n"
    "}\n";

static const char *CLOUD_FS =
    "#version 150\n"
    "uniform sampler2D Sampler0;\n"
    "uniform vec4 cloudColor;\n"
    "in vec2 texCoord;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec4 tex = texture(Sampler0, texCoord);\n"
    "    if (tex.a < 0.0392) discard;\n"
    "    fragColor = tex * cloudColor;\n"
    "}\n";

// Java: the shader compile/link helpers (the SkyRenderer pattern).
static unsigned int compile_shader(unsigned int type, const char *source)
{
    unsigned int shader = LIBMATTI_GL_glCreateShader(type);
    if (shader == 0)
        return 0;
    LIBMATTI_GL_glShaderSource(shader, source);
    LIBMATTI_GL_glCompileShader(shader);
    return shader;
}

static unsigned int ensure_cloud_program(LIBMATTI_MC_CloudRenderer *renderer)
{
    if (renderer->cloudProgram != 0)
        return renderer->cloudProgram;
    unsigned int vs = compile_shader(LIBMATTI_GL_GL_VERTEX_SHADER, CLOUD_VS);
    unsigned int fs = compile_shader(LIBMATTI_GL_GL_FRAGMENT_SHADER, CLOUD_FS);
    if (vs == 0 || fs == 0)
        return 0;
    unsigned int program = LIBMATTI_GL_glCreateProgram();
    if (program == 0)
        return 0;
    LIBMATTI_GL_glAttachShader(program, vs);
    LIBMATTI_GL_glAttachShader(program, fs);
    LIBMATTI_GL_glBindAttribLocation(program, 0, "Position");
    LIBMATTI_GL_glBindAttribLocation(program, 1, "UV0");
    LIBMATTI_GL_glLinkProgram(program);
    renderer->cloudProgram = program;
    return program;
}

// Java: the texture is assets/minecraft/textures/environment/clouds.png; the
// port uploads the generated image once.
static unsigned int ensure_cloud_texture(LIBMATTI_MC_CloudRenderer *renderer)
{
    if (renderer->cloudTexture != 0)
        return renderer->cloudTexture;
    unsigned int textures[1];
    LIBMATTI_GL_glGenTextures(1, textures);
    if (textures[0] == 0)
        return 0;
    renderer->cloudTexture = textures[0];
    LIBMATTI_GL_glBindTexture(LIBMATTI_GL_GL_TEXTURE_2D, renderer->cloudTexture);
    unsigned char *pixels = malloc((size_t) renderer->texture.width * renderer->texture.height * 4);
    if (pixels == NULL)
        return 0;
    for (long i = 0; i < (long) renderer->texture.width * renderer->texture.height; i++)
    {
        // Empty cells pack as 0 (alpha 0); populated cells carry the color.
        unsigned int argb = (unsigned int) (renderer->texture.cells[i] >> 4);
        pixels[i * 4 + 0] = (unsigned char) ((argb >> 16) & 0xFFu);
        pixels[i * 4 + 1] = (unsigned char) ((argb >> 8) & 0xFFu);
        pixels[i * 4 + 2] = (unsigned char) (argb & 0xFFu);
        pixels[i * 4 + 3] = (unsigned char) ((argb >> 24) & 0xFFu);
    }
    LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA, renderer->texture.width,
                             renderer->texture.height, 0, LIBMATTI_GL_GL_RGBA, LIBMATTI_GL_GL_UNSIGNED_BYTE, pixels);
    free(pixels);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER, LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER, LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_S, LIBMATTI_GL_GL_REPEAT);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_T, LIBMATTI_GL_GL_REPEAT);
    return renderer->cloudTexture;
}

// The VAO for the mesh (position 3f + uv 2f, the celestial layout).
static unsigned int create_cloud_vao(unsigned int vbo)
{
    unsigned int vaos[1];
    LIBMATTI_GL_glGenVertexArrays(1, vaos);
    if (vaos[0] == 0)
        return 0;
    LIBMATTI_GL_glBindVertexArray(vaos[0]);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbo);
    LIBMATTI_GL_glEnableVertexAttribArray(0);
    LIBMATTI_GL_glVertexAttribPointer(0, 3, LIBMATTI_GL_GL_FLOAT, 0, CLOUD_VERT_SIZE, (const void *) 0);
    LIBMATTI_GL_glEnableVertexAttribArray(1);
    LIBMATTI_GL_glVertexAttribPointer(1, 2, LIBMATTI_GL_GL_FLOAT, 0, CLOUD_VERT_SIZE, (const void *) 12);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    LIBMATTI_GL_glBindVertexArray(0);
    return vaos[0];
}

// ---------------------------------------------------------------------------
// Lifecycle + render
// ---------------------------------------------------------------------------

LIBMATTI_MC_CloudRenderer *LIBMATTI_MC_CloudRenderer_New(void)
{
    LIBMATTI_MC_CloudRenderer *renderer = calloc(1, sizeof(LIBMATTI_MC_CloudRenderer));
    if (renderer == NULL)
        return NULL;
    // Java: prevCellX = Integer.MIN_VALUE
    renderer->prevCellX = -2147483647 - 1;
    renderer->prevCellZ = -2147483647 - 1;
    renderer->prevRelativeCameraPos = LIBMATTI_MC_Cloud_INSIDE_CLOUDS;
    renderer->prevType = LIBMATTI_MC_CloudStatus_OFF;
    renderer->needsRebuild = 1;
    if (!prepare_texture(&renderer->texture))
    {
        free(renderer);
        return NULL;
    }
    return renderer;
}

void LIBMATTI_MC_CloudRenderer_Free(LIBMATTI_MC_CloudRenderer *renderer)
{
    if (renderer == NULL)
        return;
    free(renderer->texture.cells);
    free(renderer->meshVertices);
    if (renderer->meshVbo != 0)
        LIBMATTI_GL_glDeleteBuffers(1, &renderer->meshVbo);
    if (renderer->cloudTexture != 0)
        LIBMATTI_GL_glDeleteTextures(1, &renderer->cloudTexture);
    if (renderer->cloudProgram != 0)
        LIBMATTI_GL_glDeleteProgram(renderer->cloudProgram);
    free(renderer);
}

void LIBMATTI_MC_CloudRenderer_MarkForRebuild(LIBMATTI_MC_CloudRenderer *renderer)
{
    renderer->needsRebuild = 1;
}

// Java: public void render(int cloudColor, CloudStatus, float cloudHeight,
// Vec3 cameraPos, long gameTime, float partialTick)
void LIBMATTI_MC_CloudRenderer_Render(LIBMATTI_MC_CloudRenderer *renderer,
                                      LIBMATTI_MC_CloudStatus status,
                                      float cloudColorR, float cloudColorG, float cloudColorB,
                                      float cloudColorA,
                                      float cloudHeight,
                                      double camX, double camY, double camZ,
                                      long gameTime, float partialTick,
                                      const LIBMATTI_JOML_Matrix4f *view,
                                      const LIBMATTI_JOML_Matrix4f *projection,
                                      int cloudRangeCells)
{
    if (renderer == NULL || status == LIBMATTI_MC_CloudStatus_OFF)
        return;
    // Java: int i = options.cloudRange().get() * 16; int j = ceil(i / 12f)
    int cells = cloudRangeCells > 0 ? cloudRangeCells : 12;
    int fancy = status == LIBMATTI_MC_CloudStatus_FANCY;

    // Java: float f = cloudHeight - cameraPos.y; float f1 = f + 4.0F
    float f = cloudHeight - (float) camY;
    float f1 = f + 4.0f;
    LIBMATTI_MC_Cloud_RelativeCameraPos relativePos;
    if (f1 < 0.0f)
        relativePos = LIBMATTI_MC_Cloud_ABOVE_CLOUDS;
    else if (f > 0.0f)
        relativePos = LIBMATTI_MC_Cloud_BELOW_CLOUDS;
    else
        relativePos = LIBMATTI_MC_Cloud_INSIDE_CLOUDS;

    // Java: float f2 = gameTime % (width * 400) + partialTick
    long period = (long) renderer->texture.width * LIBMATTI_MC_Cloud_TICKS_PER_CELL;
    float f2 = (float) (gameTime % period) + partialTick;
    // Java: d0 = camX + f2 * 0.03; d1 = camZ + 3.96 - the drift + the fixed
    // z offset, wrapped into the texture's cell extent.
    double d0 = camX + f2 * 0.030000001f;
    double d1 = camZ + 3.96;
    double d2 = (double) renderer->texture.width * LIBMATTI_MC_Cloud_CELL_SIZE_IN_BLOCKS;
    double d3 = (double) renderer->texture.height * LIBMATTI_MC_Cloud_CELL_SIZE_IN_BLOCKS;
    d0 -= (double) LIBMATTI_MC_Mth_FloorD(d0 / d2) * d2;
    d1 -= (double) LIBMATTI_MC_Mth_FloorD(d1 / d3) * d3;
    int cellX = (int) LIBMATTI_MC_Mth_FloorD(d0 / LIBMATTI_MC_Cloud_CELL_SIZE_IN_BLOCKS);
    int cellZ = (int) LIBMATTI_MC_Mth_FloorD(d1 / LIBMATTI_MC_Cloud_CELL_SIZE_IN_BLOCKS);
    // Java: float f3 = d0 - cellX * 12; float f4 = d1 - cellZ * 12
    float f3 = (float) (d0 - cellX * LIBMATTI_MC_Cloud_CELL_SIZE_IN_BLOCKS);
    float f4 = (float) (d1 - cellZ * LIBMATTI_MC_Cloud_CELL_SIZE_IN_BLOCKS);

    if (renderer->needsRebuild || cellX != renderer->prevCellX || cellZ != renderer->prevCellZ
        || relativePos != renderer->prevRelativeCameraPos || status != renderer->prevType)
    {
        renderer->needsRebuild = 0;
        renderer->prevCellX = cellX;
        renderer->prevCellZ = cellZ;
        renderer->prevRelativeCameraPos = relativePos;
        renderer->prevType = status;
        build_mesh(renderer, relativePos, cellX, cellZ, fancy, cells);
        if (renderer->meshVbo == 0)
        {
            unsigned int vbos[1];
            LIBMATTI_GL_glGenBuffers(1, vbos);
            renderer->meshVbo = vbos[0];
        }
        if (renderer->meshVbo != 0 && renderer->meshVertexCount > 0)
        {
            LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, renderer->meshVbo);
            LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ARRAY_BUFFER,
                                     sizeof(float) * 5 * (size_t) renderer->meshVertexCount,
                                     renderer->meshVertices, LIBMATTI_GL_GL_DYNAMIC_DRAW);
            LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
        }
    }

    if (renderer->triangleCount == 0 || renderer->meshVbo == 0)
        return;
    unsigned int program = ensure_cloud_program(renderer);
    if (program == 0)
        return;
    unsigned int texture = ensure_cloud_texture(renderer);
    if (texture == 0)
        return;

    // The world anchor. The mesh carries the RELATIVE ring offsets, so the
    // shader origin reconstructs the base. Java's origin (-f3, f, -f4) lives
    // in the camera-relative space of its model-view; the port's view already
    // carries the full camera transform, so the origin is world space:
    // x/z = camera - the wrapped fraction, y = camera + f = the cloud height.
    float originX = (float) camX - f3;
    float originY = (float) camY + f;
    float originZ = (float) camZ - f4;

    LIBMATTI_JOML_Matrix4f mvp;
    LIBMATTI_JOML_Matrix4f_Mul(projection, view, &mvp);

    LIBMATTI_GL_glUseProgram(program);
    int mvpLocation = LIBMATTI_GL_glGetUniformLocation(program, "mvp");
    if (mvpLocation >= 0)
        LIBMATTI_GL_glUniformMatrix4fv(mvpLocation, 0, (const float *) &mvp);
    int originLocation = LIBMATTI_GL_glGetUniformLocation(program, "origin");
    if (originLocation >= 0)
        LIBMATTI_GL_glUniform3f(originLocation, originX, originY, originZ);
    int colorLocation = LIBMATTI_GL_glGetUniformLocation(program, "cloudColor");
    if (colorLocation >= 0)
        LIBMATTI_GL_glUniform4f(colorLocation, cloudColorR, cloudColorG, cloudColorB, cloudColorA);
    int samplerLocation = LIBMATTI_GL_glGetUniformLocation(program, "Sampler0");
    if (samplerLocation >= 0)
        LIBMATTI_GL_glUniform1i(samplerLocation, 0);
    LIBMATTI_B3D_GlStateManager_BindTexture(texture);

    static unsigned int cloudVao = 0;
    static unsigned int boundVbo = 0;
    if (cloudVao == 0 || boundVbo != renderer->meshVbo)
    {
        if (cloudVao != 0)
            LIBMATTI_GL_glDeleteVertexArrays(1, &cloudVao);
        cloudVao = create_cloud_vao(renderer->meshVbo);
        boundVbo = renderer->meshVbo;
    }
    if (cloudVao == 0)
        return;
    LIBMATTI_GL_glBindVertexArray(cloudVao);
    // Java: drawIndexed(0, 0, 6 * quadCount, 1) over the sequential quad
    // index buffer; the port's mesh carries the expanded triangles.
    LIBMATTI_GL_glDrawArrays(LIBMATTI_GL_GL_TRIANGLES, 0, renderer->meshVertexCount);
    LIBMATTI_GL_glBindVertexArray(0);
}

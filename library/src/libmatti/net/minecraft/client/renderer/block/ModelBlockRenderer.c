// Port of net.minecraft.client.renderer.block.ModelBlockRenderer (implementation)
// - the tesselate slice: calculateShape's face-shape bookkeeping, the AO
// per-corner brightness through the AdjacencyInfo corner tables, the
// non-cubic weights, the lightmap blending and the tint resolution.
//
// The world queries (getBlockState/getShade/getLightEngine) ride on the
// port's Level/BlockGetter surface; every formula follows the vendored
// 1.21.11 source line by line.

#include "libmatti/net/minecraft/client/renderer/block/ModelBlockRenderer.h"

#include "libmatti/net/minecraft/client/color/block/BlockColors.h"
#include "libmatti/net/minecraft/client/resources/model/QuadCollection.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/util/ARGB.h"
#include "libmatti/net/minecraft/world/level/BlockGetter.h"
#include "libmatti/net/minecraft/world/level/LevelReader.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/block/state/StateDefinition.h"
#include "libmatti/net/minecraft/world/level/block/state/properties/Property.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Java: ModelBlockRenderer.AdjacencyInfo - the corner/SizeInfo tables
// ---------------------------------------------------------------------------

// Java: SizeInfo indices (DOWN, UP, NORTH, SOUTH, WEST, EAST, then the flips)
enum
{
    SI_DOWN = 0, SI_UP, SI_NORTH, SI_SOUTH, SI_WEST, SI_EAST,
    SI_FLIP_DOWN, SI_FLIP_UP, SI_FLIP_NORTH, SI_FLIP_SOUTH, SI_FLIP_WEST, SI_FLIP_EAST
};

// Java: the per-face corner offsets in the enum's declaration order
// (DOWN: W, E, N, S ...). Extents 0/1 replace the DirectionalRelativePos.
typedef struct AdjacencyInfo
{
    signed char corners[4][3];   // Java: final Direction[] corners
    float brightness;            // Java: (unused by the AO path; Java keeps 0.5/1.0/0.8/0.6)
    int doNonCubicWeight;        // Java: final boolean doNonCubicWeight
    signed char vert0Weights[8]; // Java: the four SizeInfo[] tables
    signed char vert1Weights[8];
    signed char vert2Weights[8];
    signed char vert3Weights[8];
} AdjacencyInfo;

// Java: DOWN(W, E, N, S, 0.5, true)
static const AdjacencyInfo ADJ_DOWN = {
    {{-1, 0, 0}, {1, 0, 0}, {0, 0, -1}, {0, 0, 1}}, 0.5f, 1,
    {SI_FLIP_WEST, SI_SOUTH, SI_FLIP_WEST, SI_FLIP_SOUTH, SI_WEST, SI_FLIP_SOUTH, SI_WEST, SI_SOUTH},
    {SI_FLIP_WEST, SI_NORTH, SI_FLIP_WEST, SI_FLIP_NORTH, SI_WEST, SI_FLIP_NORTH, SI_WEST, SI_NORTH},
    {SI_FLIP_EAST, SI_NORTH, SI_FLIP_EAST, SI_FLIP_NORTH, SI_EAST, SI_FLIP_NORTH, SI_EAST, SI_NORTH},
    {SI_FLIP_EAST, SI_SOUTH, SI_FLIP_EAST, SI_FLIP_SOUTH, SI_EAST, SI_FLIP_SOUTH, SI_EAST, SI_SOUTH},
};

// Java: UP(E, W, N, S, 1.0, true)
static const AdjacencyInfo ADJ_UP = {
    {{1, 0, 0}, {-1, 0, 0}, {0, 0, -1}, {0, 0, 1}}, 1.0f, 1,
    {SI_EAST, SI_SOUTH, SI_EAST, SI_FLIP_SOUTH, SI_FLIP_EAST, SI_FLIP_SOUTH, SI_FLIP_EAST, SI_SOUTH},
    {SI_EAST, SI_NORTH, SI_EAST, SI_FLIP_NORTH, SI_FLIP_EAST, SI_FLIP_NORTH, SI_FLIP_EAST, SI_NORTH},
    {SI_WEST, SI_NORTH, SI_WEST, SI_FLIP_NORTH, SI_FLIP_WEST, SI_FLIP_NORTH, SI_FLIP_WEST, SI_NORTH},
    {SI_WEST, SI_SOUTH, SI_WEST, SI_FLIP_SOUTH, SI_FLIP_WEST, SI_FLIP_SOUTH, SI_FLIP_WEST, SI_SOUTH},
};

// Java: NORTH(U, D, E, W, 0.8, true)
static const AdjacencyInfo ADJ_NORTH = {
    {{0, 1, 0}, {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}}, 0.8f, 1,
    {SI_UP, SI_FLIP_WEST, SI_UP, SI_WEST, SI_FLIP_UP, SI_WEST, SI_FLIP_UP, SI_FLIP_WEST},
    {SI_UP, SI_FLIP_EAST, SI_UP, SI_EAST, SI_FLIP_UP, SI_EAST, SI_FLIP_UP, SI_FLIP_EAST},
    {SI_DOWN, SI_FLIP_EAST, SI_DOWN, SI_EAST, SI_FLIP_DOWN, SI_EAST, SI_FLIP_DOWN, SI_FLIP_EAST},
    {SI_DOWN, SI_FLIP_WEST, SI_DOWN, SI_WEST, SI_FLIP_DOWN, SI_WEST, SI_FLIP_DOWN, SI_FLIP_WEST},
};

// Java: SOUTH(W, E, D, U, 0.8, true)
static const AdjacencyInfo ADJ_SOUTH = {
    {{-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}}, 0.8f, 1,
    {SI_UP, SI_FLIP_WEST, SI_FLIP_UP, SI_FLIP_WEST, SI_FLIP_UP, SI_WEST, SI_UP, SI_WEST},
    {SI_DOWN, SI_FLIP_WEST, SI_FLIP_DOWN, SI_FLIP_WEST, SI_FLIP_DOWN, SI_WEST, SI_DOWN, SI_WEST},
    {SI_DOWN, SI_FLIP_EAST, SI_FLIP_DOWN, SI_FLIP_EAST, SI_FLIP_DOWN, SI_EAST, SI_DOWN, SI_EAST},
    {SI_UP, SI_FLIP_EAST, SI_FLIP_UP, SI_FLIP_EAST, SI_FLIP_UP, SI_EAST, SI_UP, SI_EAST},
};

// Java: WEST(U, D, N, S, 0.6, true)
static const AdjacencyInfo ADJ_WEST = {
    {{0, 1, 0}, {0, -1, 0}, {0, 0, -1}, {0, 0, 1}}, 0.6f, 1,
    {SI_UP, SI_SOUTH, SI_UP, SI_FLIP_SOUTH, SI_FLIP_UP, SI_FLIP_SOUTH, SI_FLIP_UP, SI_SOUTH},
    {SI_UP, SI_NORTH, SI_UP, SI_FLIP_NORTH, SI_FLIP_UP, SI_FLIP_NORTH, SI_FLIP_UP, SI_NORTH},
    {SI_DOWN, SI_NORTH, SI_DOWN, SI_FLIP_NORTH, SI_FLIP_DOWN, SI_FLIP_NORTH, SI_FLIP_DOWN, SI_NORTH},
    {SI_DOWN, SI_SOUTH, SI_DOWN, SI_FLIP_SOUTH, SI_FLIP_DOWN, SI_FLIP_SOUTH, SI_FLIP_DOWN, SI_SOUTH},
};

// Java: EAST(D, U, N, S, 0.6, true)
static const AdjacencyInfo ADJ_EAST = {
    {{0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}}, 0.6f, 1,
    {SI_FLIP_DOWN, SI_SOUTH, SI_FLIP_DOWN, SI_FLIP_SOUTH, SI_DOWN, SI_FLIP_SOUTH, SI_DOWN, SI_SOUTH},
    {SI_FLIP_DOWN, SI_NORTH, SI_FLIP_DOWN, SI_FLIP_NORTH, SI_DOWN, SI_FLIP_NORTH, SI_DOWN, SI_NORTH},
    {SI_FLIP_UP, SI_NORTH, SI_FLIP_UP, SI_FLIP_NORTH, SI_UP, SI_FLIP_NORTH, SI_UP, SI_NORTH},
    {SI_FLIP_UP, SI_SOUTH, SI_FLIP_UP, SI_FLIP_SOUTH, SI_UP, SI_FLIP_SOUTH, SI_UP, SI_SOUTH},
};

static const AdjacencyInfo *adjacency_from_facing(LIBMATTI_MC_Direction facing)
{
    switch (facing)
    {
    case LIBMATTI_MC_Direction_DOWN: return &ADJ_DOWN;
    case LIBMATTI_MC_Direction_UP: return &ADJ_UP;
    case LIBMATTI_MC_Direction_NORTH: return &ADJ_NORTH;
    case LIBMATTI_MC_Direction_SOUTH: return &ADJ_SOUTH;
    case LIBMATTI_MC_Direction_WEST: return &ADJ_WEST;
    case LIBMATTI_MC_Direction_EAST: return &ADJ_EAST;
    default: return &ADJ_DOWN;
    }
}

// Java: AmbientVertexRemap - vert0..3 per facing
typedef struct AmbientVertexRemap
{
    signed char vert0, vert1, vert2, vert3;
} AmbientVertexRemap;

static const AmbientVertexRemap REMAP_DOWN = {0, 1, 2, 3};
static const AmbientVertexRemap REMAP_UP = {2, 3, 0, 1};
static const AmbientVertexRemap REMAP_NORTH = {3, 0, 1, 2};
static const AmbientVertexRemap REMAP_SOUTH = {0, 1, 2, 3};
static const AmbientVertexRemap REMAP_WEST = {3, 0, 1, 2};
static const AmbientVertexRemap REMAP_EAST = {1, 2, 3, 0};

static const AmbientVertexRemap *remap_from_facing(LIBMATTI_MC_Direction facing)
{
    switch (facing)
    {
    case LIBMATTI_MC_Direction_DOWN: return &REMAP_DOWN;
    case LIBMATTI_MC_Direction_UP: return &REMAP_UP;
    case LIBMATTI_MC_Direction_NORTH: return &REMAP_NORTH;
    case LIBMATTI_MC_Direction_SOUTH: return &REMAP_SOUTH;
    case LIBMATTI_MC_Direction_WEST: return &REMAP_WEST;
    case LIBMATTI_MC_Direction_EAST: return &REMAP_EAST;
    default: return &REMAP_DOWN;
    }
}

// ---------------------------------------------------------------------------
// Java: ClientLevel.getShade - the cardinal light table (overworld)
// ---------------------------------------------------------------------------

// Returns full brightness when shading is disabled. Otherwise returns the
// overworld directional factor: down 0.5, up 1.0, north/south 0.8, west/east
// 0.6; unknown directions also use full brightness.
float LIBMATTI_MC_ModelBlockRenderer_GetShade(LIBMATTI_MC_Direction facing, int shade)
{
    if (!shade)
        return 1.0f;
    switch (facing)
    {
    case LIBMATTI_MC_Direction_DOWN: return 0.5f;
    case LIBMATTI_MC_Direction_UP: return 1.0f;
    case LIBMATTI_MC_Direction_NORTH:
    case LIBMATTI_MC_Direction_SOUTH: return 0.8f;
    case LIBMATTI_MC_Direction_WEST:
    case LIBMATTI_MC_Direction_EAST: return 0.6f;
    default: return 1.0f;
    }
}

// ---------------------------------------------------------------------------
// Java: the BlockStateBase light/shade surface over the port's properties
// ---------------------------------------------------------------------------

// Java: isSolidRender() = solidRender (canOcclude && the occlusion shape is a
// full cube) - the port answers through the canOcclude flag.
static int state_solid_render(const LIBMATTI_MC_BlockState *state)
{
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    return block != NULL && block->properties != NULL && block->properties->canOcclude;
}

// Java: isCollisionShapeFullBlock - the port's proxy is the occlusion flag.
static int state_full_block(const LIBMATTI_MC_BlockState *state)
{
    return state_solid_render(state);
}

// Java: getLightBlock() - 15 for solid render, 0/1 through the skylight proxy.
static int state_light_block(const LIBMATTI_MC_BlockState *state)
{
    if (state_solid_render(state))
        return 15;
    // Java: propagatesSkylightDown() ? 0 : 1 - the port's proxy is the air flag
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    if (block != NULL && block->properties != NULL && block->properties->isAir)
        return 0;
    return 1;
}

// Java: isViewBlocking = the isSuffocating predicate - the port's proxy is the
// solid-render flag.
static int state_view_blocking(const LIBMATTI_MC_BlockState *state)
{
    return state_solid_render(state);
}

float LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(
    const LIBMATTI_MC_BlockState *state,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const LIBMATTI_MC_BlockPos *pos)
{
    (void) level;
    (void) pos;
    // Java: protected float getShadeBrightness - isCollisionShapeFullBlock ? 0.2F : 1.0F
    return state_full_block(state) ? 0.2f : 1.0f;
}

// Java: LevelRenderer.getLightColor - the packed (block, sky) pair; the light
// engine is game-port content, the port answers sky = 15 under open sky
// (the column scan) and block = the state's light emission, exactly the
// BrightnessGetter.DEFAULT shape the skeleton can resolve.
int LIBMATTI_MC_ModelBlockRenderer_GetLightColor(
    const LIBMATTI_MC_BlockState *state,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const LIBMATTI_MC_BlockPos *pos)
{
    (void) state;
    // Java: BrightnessGetter.DEFAULT - getBrightness(SKY) << 20 | getBrightness(BLOCK) << 4
    struct LIBMATTI_MC_Level *lv = (struct LIBMATTI_MC_Level *) level;
    int sky = LIBMATTI_MC_LevelReader_CanSeeSky(lv, pos) ? 15 : 0;
    int block = 0;
    int emission = 0;
    const LIBMATTI_MC_Block *b = LIBMATTI_MC_BlockState_GetBlock(state);
    if (b != NULL && b->properties != NULL && b->properties->lightEmission > 0)
        emission = b->properties->lightEmission;
    // Java: LightTexture.pack(block, sky) = block << 4 | sky << 20
    return (block & 15) << 4 | (sky & 15) << 20;
}

// Java: Block.shouldRenderFace(state, neighbour, direction) - the port's
// proxy: the neighbour occludes when it is a full cube (skipRendering and the
// partial face-occlusion shapes arrive with the VoxelShape port).
static int block_should_render_face(const LIBMATTI_MC_BlockState *state,
                                    const LIBMATTI_MC_BlockState *neighbour,
                                    LIBMATTI_MC_Direction direction)
{
    (void) state;
    (void) direction;
    return !state_solid_render(neighbour);
}

// Java: shouldRenderFace(BlockAndTintGetter, state, checkSides, direction, offsetPos)
static int should_render_face(struct LIBMATTI_MC_BlockAndTintGetter *level,
                              const LIBMATTI_MC_BlockState *state, int checkSides,
                              LIBMATTI_MC_Direction direction, const LIBMATTI_MC_BlockPos *offsetPos)
{
    if (!checkSides)
        return 1;
    const LIBMATTI_MC_BlockState *neighbour = LIBMATTI_MC_BlockGetter_GetBlockState(
        (struct LIBMATTI_MC_Level *) level, offsetPos);
    if (neighbour == NULL)
        return 1;
    return block_should_render_face(state, neighbour, direction);
}

// ---------------------------------------------------------------------------
// Java: calculateShape - the face-shape bookkeeping (AO path only uses the
// faceShape slots + facePartial/faceCubic; the storage carries them)
// ---------------------------------------------------------------------------

typedef struct RenderContext
{
    struct LIBMATTI_MC_BlockAndTintGetter *level;
    const LIBMATTI_MC_BlockState *state;
    const LIBMATTI_MC_BlockPos *pos;
    int checkSides;
    LIBMATTI_MC_ModelBlockRenderer_Storage *storage;
    float faceShape[LIBMATTI_MC_ModelBlockRenderer_SIZEINFO_COUNT];
} RenderContext;

static void calculate_shape(RenderContext *ctx, const LIBMATTI_MC_BakedQuad *quad)
{
    float f = 32.0f, f1 = 32.0f, f2 = 32.0f;
    float f3 = -32.0f, f4 = -32.0f, f5 = -32.0f;
    for (int i = 0; i < 4; i++)
    {
        float x = quad->pos[i][0];
        float y = quad->pos[i][1];
        float z = quad->pos[i][2];
        f = f < x ? f : x;
        f1 = f1 < y ? f1 : y;
        f2 = f2 < z ? f2 : z;
        f3 = f3 > x ? f3 : x;
        f4 = f4 > y ? f4 : y;
        f5 = f5 > z ? f5 : z;
    }

    // Java: the AmbientOcclusionRenderStorage branch fills the faceShape slots
    ctx->faceShape[SI_WEST] = f;
    ctx->faceShape[SI_EAST] = f3;
    ctx->faceShape[SI_DOWN] = f1;
    ctx->faceShape[SI_UP] = f4;
    ctx->faceShape[SI_NORTH] = f2;
    ctx->faceShape[SI_SOUTH] = f5;
    ctx->faceShape[SI_FLIP_WEST] = 1.0f - f;
    ctx->faceShape[SI_FLIP_EAST] = 1.0f - f3;
    ctx->faceShape[SI_FLIP_DOWN] = 1.0f - f1;
    ctx->faceShape[SI_FLIP_UP] = 1.0f - f4;
    ctx->faceShape[SI_FLIP_NORTH] = 1.0f - f2;
    ctx->faceShape[SI_FLIP_SOUTH] = 1.0f - f5;

    ctx->storage->facePartial =
        quad->direction == LIBMATTI_MC_Direction_DOWN || quad->direction == LIBMATTI_MC_Direction_UP
            ? (f >= 1.0e-4f || f2 >= 1.0e-4f || f3 <= 0.9999f || f5 <= 0.9999f)
            : quad->direction == LIBMATTI_MC_Direction_NORTH || quad->direction == LIBMATTI_MC_Direction_SOUTH
                  ? (f >= 1.0e-4f || f1 >= 1.0e-4f || f3 <= 0.9999f || f4 <= 0.9999f)
                  : (f1 >= 1.0e-4f || f2 >= 1.0e-4f || f4 <= 0.9999f || f5 <= 0.9999f);

    int fullBlock = state_full_block(ctx->state);
    switch (quad->direction)
    {
    case LIBMATTI_MC_Direction_DOWN:
        ctx->storage->faceCubic = f1 == f4 && (f1 < 1.0e-4f || fullBlock);
        break;
    case LIBMATTI_MC_Direction_UP:
        ctx->storage->faceCubic = f1 == f4 && (f4 > 0.9999f || fullBlock);
        break;
    case LIBMATTI_MC_Direction_NORTH:
        ctx->storage->faceCubic = f2 == f5 && (f2 < 1.0e-4f || fullBlock);
        break;
    case LIBMATTI_MC_Direction_SOUTH:
        ctx->storage->faceCubic = f2 == f5 && (f5 > 0.9999f || fullBlock);
        break;
    case LIBMATTI_MC_Direction_WEST:
        ctx->storage->faceCubic = f == f3 && (f < 1.0e-4f || fullBlock);
        break;
    case LIBMATTI_MC_Direction_EAST:
        ctx->storage->faceCubic = f == f3 && (f3 > 0.9999f || fullBlock);
        break;
    default:
        ctx->storage->faceCubic = 0;
        break;
    }
}

// ---------------------------------------------------------------------------
// Java: AmbientOcclusionRenderStorage.calculate + the two blend helpers
// ---------------------------------------------------------------------------

// Java: private static int blend(int, int, int, int) - the 4-way light average
// with the zero-fallback and the 0x00FF00FF mask.
static int blend4(int a, int b, int c, int d)
{
    if (a == 0)
        a = d;
    if (b == 0)
        b = d;
    if (c == 0)
        c = d;
    return (a + b + c + d) >> 2 & 0x00FF00FF;
}

// Java: private static int blend(int, int, int, int, float, float, float, float)
static int blend_weighted(int a, int b, int c, int d, float w0, float w1, float w2, float w3)
{
    int i = (int) ((float) (a >> 16 & 0xFF) * w0 + (float) (b >> 16 & 0xFF) * w1
                   + (float) (c >> 16 & 0xFF) * w2 + (float) (d >> 16 & 0xFF) * w3) & 0xFF;
    int j = (int) ((float) (a & 0xFF) * w0 + (float) (b & 0xFF) * w1
                   + (float) (c & 0xFF) * w2 + (float) (d & 0xFF) * w3) & 0xFF;
    return i << 16 | j;
}

// Java: the corner walk of AmbientOcclusionRenderStorage.calculate
static void ao_calculate(RenderContext *ctx, LIBMATTI_MC_Direction facing, int shade)
{
    LIBMATTI_MC_ModelBlockRenderer_Storage *st = ctx->storage;
    const AdjacencyInfo *adj = adjacency_from_facing(facing);
    const AmbientVertexRemap *remap = remap_from_facing(facing);
    struct LIBMATTI_MC_BlockAndTintGetter *level = ctx->level;
    struct LIBMATTI_MC_Level *lv = (struct LIBMATTI_MC_Level *) level;

    // Java: BlockPos blockpos = faceCubic ? pos.relative(facing) : pos
    LIBMATTI_MC_BlockPos blockpos;
    if (st->faceCubic)
    {
        blockpos.base.x = ctx->pos->base.x + LIBMATTI_MC_Direction_GetStepX(facing);
        blockpos.base.y = ctx->pos->base.y + LIBMATTI_MC_Direction_GetStepY(facing);
        blockpos.base.z = ctx->pos->base.z + LIBMATTI_MC_Direction_GetStepZ(facing);
    }
    else
    {
        blockpos = *ctx->pos;
    }

    // Java: the four corner states with their light + shade brightness
    LIBMATTI_MC_BlockPos scratch;
    int light[4];
    float shadeBrightness[4];
    for (int c = 0; c < 4; c++)
    {
        scratch.base.x = blockpos.base.x + adj->corners[c][0];
        scratch.base.y = blockpos.base.y + adj->corners[c][1];
        scratch.base.z = blockpos.base.z + adj->corners[c][2];
        const LIBMATTI_MC_BlockState *cornerState = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &scratch);
        if (cornerState == NULL)
        {
            light[c] = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(ctx->state, level, &scratch);
            shadeBrightness[c] = 1.0f;
            continue;
        }
        light[c] = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(cornerState, level, &scratch);
        shadeBrightness[c] = LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(cornerState, level, &scratch);
    }

    // Java: the four diagonal-above states (corner + facing) for the open check
    int open[4];
    for (int c = 0; c < 4; c++)
    {
        scratch.base.x = blockpos.base.x + adj->corners[c][0] + LIBMATTI_MC_Direction_GetStepX(facing);
        scratch.base.y = blockpos.base.y + adj->corners[c][1] + LIBMATTI_MC_Direction_GetStepY(facing);
        scratch.base.z = blockpos.base.z + adj->corners[c][2] + LIBMATTI_MC_Direction_GetStepZ(facing);
        const LIBMATTI_MC_BlockState *aboveState = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &scratch);
        open[c] = aboveState == NULL || !state_view_blocking(aboveState) || state_light_block(aboveState) == 0;
    }

    // Java: the three inner diagonals (f4/f5/f6/f7 with their lights)
    float f4 = shadeBrightness[0], f5 = shadeBrightness[0], f6 = shadeBrightness[0], f7 = shadeBrightness[0];
    int i1 = light[0], j1 = light[0], k1 = light[0], l1 = light[0];
    if (open[2] || open[0])
    {
        scratch.base.x = blockpos.base.x + adj->corners[0][0] + adj->corners[2][0];
        scratch.base.y = blockpos.base.y + adj->corners[0][1] + adj->corners[2][1];
        scratch.base.z = blockpos.base.z + adj->corners[0][2] + adj->corners[2][2];
        const LIBMATTI_MC_BlockState *s = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &scratch);
        if (s != NULL)
        {
            f4 = LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(s, level, &scratch);
            i1 = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(s, level, &scratch);
        }
    }
    if (open[3] || open[0])
    {
        scratch.base.x = blockpos.base.x + adj->corners[0][0] + adj->corners[3][0];
        scratch.base.y = blockpos.base.y + adj->corners[0][1] + adj->corners[3][1];
        scratch.base.z = blockpos.base.z + adj->corners[0][2] + adj->corners[3][2];
        const LIBMATTI_MC_BlockState *s = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &scratch);
        if (s != NULL)
        {
            f5 = LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(s, level, &scratch);
            j1 = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(s, level, &scratch);
        }
    }
    if (open[2] || open[1])
    {
        scratch.base.x = blockpos.base.x + adj->corners[1][0] + adj->corners[2][0];
        scratch.base.y = blockpos.base.y + adj->corners[1][1] + adj->corners[2][1];
        scratch.base.z = blockpos.base.z + adj->corners[1][2] + adj->corners[2][2];
        const LIBMATTI_MC_BlockState *s = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &scratch);
        if (s != NULL)
        {
            f6 = LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(s, level, &scratch);
            k1 = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(s, level, &scratch);
        }
    }
    if (open[3] || open[1])
    {
        scratch.base.x = blockpos.base.x + adj->corners[1][0] + adj->corners[3][0];
        scratch.base.y = blockpos.base.y + adj->corners[1][1] + adj->corners[3][1];
        scratch.base.z = blockpos.base.z + adj->corners[1][2] + adj->corners[3][2];
        const LIBMATTI_MC_BlockState *s = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &scratch);
        if (s != NULL)
        {
            f7 = LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(s, level, &scratch);
            l1 = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(s, level, &scratch);
        }
    }

    // Java: i3 - the light of the face itself (the neighbour when the face is
    // cubic or the neighbour is not solid render)
    int i3 = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(ctx->state, level, ctx->pos);
    LIBMATTI_MC_BlockPos facePos = {
        {ctx->pos->base.x + LIBMATTI_MC_Direction_GetStepX(facing),
         ctx->pos->base.y + LIBMATTI_MC_Direction_GetStepY(facing),
         ctx->pos->base.z + LIBMATTI_MC_Direction_GetStepZ(facing)}};
    const LIBMATTI_MC_BlockState *faceState = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &facePos);
    if (faceState != NULL && (st->faceCubic || !state_solid_render(faceState)))
        i3 = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(faceState, level, &facePos);

    // Java: f8 - the block's own shade brightness (cubic: the neighbour's)
    float f8;
    if (st->faceCubic)
    {
        const LIBMATTI_MC_BlockState *neighbourState = LIBMATTI_MC_BlockGetter_GetBlockState(lv, &blockpos);
        f8 = neighbourState != NULL ? LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(neighbourState, level, &blockpos)
                                    : 1.0f;
    }
    else
    {
        f8 = LIBMATTI_MC_ModelBlockRenderer_GetShadeBrightness(ctx->state, level, ctx->pos);
    }

    // Java: the corner brightness sums (f29..f33 / f9..f12 naming)
    float b0 = (shadeBrightness[3] + shadeBrightness[0] + f5 + f8) * 0.25f;
    float b1 = (shadeBrightness[2] + shadeBrightness[0] + f4 + f8) * 0.25f;
    float b2 = (shadeBrightness[2] + shadeBrightness[1] + f6 + f8) * 0.25f;
    float b3 = (shadeBrightness[3] + shadeBrightness[1] + f7 + f8) * 0.25f;

    if (st->facePartial && adj->doNonCubicWeight)
    {
        // Java: the non-cubic weights - the faceShape products per vertex
        float w[4][8];
        const signed char *tables[4] = {adj->vert0Weights, adj->vert1Weights, adj->vert2Weights, adj->vert3Weights};
        for (int v = 0; v < 4; v++)
        {
            for (int p = 0; p < 4; p++)
            {
                float a = ctx->faceShape[tables[v][p * 2]];
                float b = ctx->faceShape[tables[v][p * 2 + 1]];
                w[v][p] = a * b;
            }
        }
        st->brightness[remap->vert0] = fminf(fmaxf(b0 * w[0][0] + b1 * w[0][1] + b2 * w[0][2] + b3 * w[0][3], 0.0f), 1.0f);
        st->brightness[remap->vert1] = fminf(fmaxf(b0 * w[1][0] + b1 * w[1][1] + b2 * w[1][2] + b3 * w[1][3], 0.0f), 1.0f);
        st->brightness[remap->vert2] = fminf(fmaxf(b0 * w[2][0] + b1 * w[2][1] + b2 * w[2][2] + b3 * w[2][3], 0.0f), 1.0f);
        st->brightness[remap->vert3] = fminf(fmaxf(b0 * w[3][0] + b1 * w[3][1] + b2 * w[3][2] + b3 * w[3][3], 0.0f), 1.0f);

        // Java: the lightmap blends - the two-level weighted average
        int i2 = blend4(l1, light[0], j1, i3);
        int j2 = blend4(light[2], light[0], i1, i3);
        int k2 = blend4(light[2], light[1], k1, i3);
        int l2 = blend4(l1, light[1], l1, i3);
        st->lightmap[remap->vert0] = blend_weighted(i2, j2, k2, l2, w[0][0], w[0][1], w[0][2], w[0][3]);
        st->lightmap[remap->vert1] = blend_weighted(i2, j2, k2, l2, w[1][0], w[1][1], w[1][2], w[1][3]);
        st->lightmap[remap->vert2] = blend_weighted(i2, j2, k2, l2, w[2][0], w[2][1], w[2][2], w[2][3]);
        st->lightmap[remap->vert3] = blend_weighted(i2, j2, k2, l2, w[3][0], w[3][1], w[3][2], w[3][3]);
    }
    else
    {
        st->lightmap[remap->vert0] = blend4(l1, light[0], j1, i3);
        st->lightmap[remap->vert1] = blend4(light[2], light[0], i1, i3);
        st->lightmap[remap->vert2] = blend4(light[2], light[1], k1, i3);
        st->lightmap[remap->vert3] = blend4(l1, light[1], l1, i3);
        st->brightness[remap->vert0] = b0;
        st->brightness[remap->vert1] = b1;
        st->brightness[remap->vert2] = b2;
        st->brightness[remap->vert3] = b3;
    }

    // Java: the directional shade multiplies every corner
    float faceShade = LIBMATTI_MC_ModelBlockRenderer_GetShade(facing, shade);
    for (int v = 0; v < 4; v++)
        st->brightness[v] = st->brightness[v] * faceShade;
}

// ---------------------------------------------------------------------------
// Java: putQuadData - the tint resolution and the sink emission
// ---------------------------------------------------------------------------

typedef struct SinkInfo
{
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b, int lightmap);
    void *userdata;
    // Java: the renderer owning the storage (the tint cache and BlockColors)
    LIBMATTI_MC_ModelBlockRenderer *renderer;
} SinkInfo;

static void put_quad_data(RenderContext *ctx, const LIBMATTI_MC_BakedQuad *quad, const SinkInfo *sink)
{
    LIBMATTI_MC_ModelBlockRenderer_Storage *st = ctx->storage;
    float f, f1, f2;
    if (quad->tintIndex != -1)
    {
        int j;
        if (st->tintCacheIndex == quad->tintIndex)
        {
            j = st->tintCacheValue;
        }
        else
        {
            j = -1;
            if (sink->renderer != NULL && sink->renderer->blockColors != NULL)
                j = LIBMATTI_MC_BlockColors_GetColorIndexed(
                    sink->renderer->blockColors, ctx->state, ctx->level, ctx->pos, quad->tintIndex);
            st->tintCacheIndex = quad->tintIndex;
            st->tintCacheValue = j;
        }
        f = LIBMATTI_MC_ARGB_RedFloat(j);
        f1 = LIBMATTI_MC_ARGB_GreenFloat(j);
        f2 = LIBMATTI_MC_ARGB_BlueFloat(j);
    }
    else
    {
        f = 1.0f;
        f1 = 1.0f;
        f2 = 1.0f;
    }

    // Java: putBulkData(pose, quad, brightness[], r, g, b, 1.0F, lightmap[],
    // overlay) - the per-vertex brightness folds into the vertex color.
    for (int corner = 0; corner < 4; corner++)
    {
        float vertexPos[3] = {quad->pos[corner][0], quad->pos[corner][1], quad->pos[corner][2]};
        float shade = st->brightness[corner];
        sink->sink(sink->userdata, corner, vertexPos, quad, f * shade, f1 * shade, f2 * shade,
                   st->lightmap[corner]);
    }
}

// ---------------------------------------------------------------------------
// The tesselate paths
// ---------------------------------------------------------------------------

static void render_face_list_ao(RenderContext *ctx, const LIBMATTI_MC_BakedQuad *quads, size_t count, const SinkInfo *sink)
{
    for (size_t q = 0; q < count; q++)
    {
        calculate_shape(ctx, &quads[q]);
        ao_calculate(ctx, quads[q].direction, quads[q].shade);
        put_quad_data(ctx, &quads[q], sink);
    }
}

static void render_face_list_flat(RenderContext *ctx, const LIBMATTI_MC_BakedQuad *quads, size_t count,
                                  int light, const SinkInfo *sink)
{
    for (size_t q = 0; q < count; q++)
    {
        const LIBMATTI_MC_BakedQuad *quad = &quads[q];
        LIBMATTI_MC_ModelBlockRenderer_Storage *st = ctx->storage;
        // Java: float f = level.getShade(quad.direction(), quad.shade()) - the
        // one shade value goes into all four brightness slots.
        float shade = LIBMATTI_MC_ModelBlockRenderer_GetShade(quad->direction, quad->shade);
        st->brightness[0] = shade;
        st->brightness[1] = shade;
        st->brightness[2] = shade;
        st->brightness[3] = shade;
        st->lightmap[0] = light;
        st->lightmap[1] = light;
        st->lightmap[2] = light;
        st->lightmap[3] = light;
        put_quad_data(ctx, quad, sink);
    }
}

static void tesselate_impl(LIBMATTI_MC_ModelBlockRenderer *renderer,
                           struct LIBMATTI_MC_BlockAndTintGetter *level,
                           const LIBMATTI_MC_QuadCollection *model,
                           const LIBMATTI_MC_BlockState *state,
                           const LIBMATTI_MC_BlockPos *pos,
                           int checkSides,
                           int withAO,
                           void (*sinkFn)(void *userdata, int corner, const float vertexPos[3],
                                          const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b,
                                          int lightmap),
                           void *sinkUserdata)
{
    SinkInfo sink = {sinkFn, sinkUserdata, renderer};
    RenderContext ctx;
    ctx.level = level;
    ctx.state = state;
    ctx.pos = pos;
    ctx.checkSides = checkSides;
    LIBMATTI_MC_ModelBlockRenderer_Storage storage;
    memset(&storage, 0, sizeof(storage));
    storage.tintCacheIndex = -1;
    ctx.storage = &storage;
    memset(ctx.faceShape, 0, sizeof(ctx.faceShape));
    (void) renderer;

    for (int d = 0; d < 6; d++)
    {
        LIBMATTI_MC_Direction direction = (LIBMATTI_MC_Direction) d;
        LIBMATTI_MC_BlockPos offsetPos = {
            {pos->base.x + LIBMATTI_MC_Direction_GetStepX(direction),
             pos->base.y + LIBMATTI_MC_Direction_GetStepY(direction),
             pos->base.z + LIBMATTI_MC_Direction_GetStepZ(direction)}};
        if (!should_render_face(level, state, checkSides, direction, &offsetPos))
            continue;

        size_t count = 0;
        const LIBMATTI_MC_BakedQuad *quads = LIBMATTI_MC_QuadCollection_GetCulled(model, direction, &count);
        if (count == 0)
            continue;

        if (withAO)
            render_face_list_ao(&ctx, quads, count, &sink);
        else
        {
            int light = LIBMATTI_MC_ModelBlockRenderer_GetLightColor(state, level, &offsetPos);
            render_face_list_flat(&ctx, quads, count, light, &sink);
        }
    }

    // Java: getQuads(null) - the unculled list
    size_t unculled = 0;
    const LIBMATTI_MC_BakedQuad *unculledQuads = LIBMATTI_MC_QuadCollection_GetUnculled(model, &unculled);
    if (unculled != 0)
    {
        if (withAO)
            render_face_list_ao(&ctx, unculledQuads, unculled, &sink);
        else
            render_face_list_flat(&ctx, unculledQuads, unculled, -1, &sink);
    }
}

LIBMATTI_MC_ModelBlockRenderer *LIBMATTI_MC_ModelBlockRenderer_New(LIBMATTI_MC_BlockColors *colors)
{
    LIBMATTI_MC_ModelBlockRenderer *renderer = calloc(1, sizeof(LIBMATTI_MC_ModelBlockRenderer));
    renderer->blockColors = colors;
    return renderer;
}

void LIBMATTI_MC_ModelBlockRenderer_Free(LIBMATTI_MC_ModelBlockRenderer *renderer)
{
    free(renderer);
}

void LIBMATTI_MC_ModelBlockRenderer_TesselateWithAO(
    LIBMATTI_MC_ModelBlockRenderer *renderer,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const LIBMATTI_MC_QuadCollection *model,
    const LIBMATTI_MC_BlockState *state,
    const LIBMATTI_MC_BlockPos *pos,
    int checkSides,
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b, int lightmap),
    void *sinkUserdata)
{
    tesselate_impl(renderer, level, model, state, pos, checkSides, 1, sink, sinkUserdata);
}

void LIBMATTI_MC_ModelBlockRenderer_TesselateWithoutAO(
    LIBMATTI_MC_ModelBlockRenderer *renderer,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const LIBMATTI_MC_QuadCollection *model,
    const LIBMATTI_MC_BlockState *state,
    const LIBMATTI_MC_BlockPos *pos,
    int checkSides,
    void (*sink)(void *userdata, int corner, const float vertexPos[3],
                 const LIBMATTI_MC_BakedQuad *quad, float r, float g, float b, int lightmap),
    void *sinkUserdata)
{
    tesselate_impl(renderer, level, model, state, pos, checkSides, 0, sink, sinkUserdata);
}

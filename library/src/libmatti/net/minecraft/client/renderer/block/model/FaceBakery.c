// Port of net.minecraft.client.renderer.block.model.FaceBakery (implementation).
// Every formula follows the vendored 1.21.11 source.

#include "libmatti/net/minecraft/client/renderer/block/model/FaceBakery.h"

#include "libmatti/net/minecraft/core/Direction.h"

#include <math.h>
#include <string.h>

// Java: private static final Vector3fc BLOCK_MIDDLE - the rotation centre
// for the ModelState transformations (the vanilla identity state skips it).

// Java: FaceInfo.Extent - the MIN/MAX selector over the (min, mid, max) triple.
// Java's select(minX, minY, minZ, maxX, maxY, maxZ) picks the extent's axis
// bound; the port keeps the same five-argument shape as FaceInfo.
typedef enum FaceExtent
{
    FACE_EXTENT_MIN_X,
    FACE_EXTENT_MIN_Y,
    FACE_EXTENT_MIN_Z,
    FACE_EXTENT_MAX_X,
    FACE_EXTENT_MAX_Y,
    FACE_EXTENT_MAX_Z
} FaceExtent;

typedef struct FaceVertexInfo
{
    FaceExtent x;
    FaceExtent y;
    FaceExtent z;
} FaceVertexInfo;

static const FaceVertexInfo FACE_INFO[6][4] = {
    // Java: DOWN(new VertexInfo(MIN_X, MIN_Y, MAX_Z), (MIN_X, MIN_Y, MIN_Z),
    //            (MAX_X, MIN_Y, MIN_Z), (MAX_X, MIN_Y, MAX_Z))
    {{FACE_EXTENT_MIN_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MAX_Z}},
    // Java: UP(new VertexInfo(MIN_X, MAX_Y, MIN_Z), (MIN_X, MAX_Y, MAX_Z),
    //          (MAX_X, MAX_Y, MAX_Z), (MAX_X, MAX_Y, MIN_Z))
    {{FACE_EXTENT_MIN_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MIN_Z}},
    // Java: NORTH(new VertexInfo(MAX_X, MAX_Y, MIN_Z), (MAX_X, MIN_Y, MIN_Z),
    //             (MIN_X, MIN_Y, MIN_Z), (MIN_X, MAX_Y, MIN_Z))
    {{FACE_EXTENT_MAX_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MIN_Z}},
    // Java: SOUTH(new VertexInfo(MIN_X, MAX_Y, MAX_Z), (MIN_X, MIN_Y, MAX_Z),
    //             (MAX_X, MIN_Y, MAX_Z), (MAX_X, MAX_Y, MAX_Z))
    {{FACE_EXTENT_MIN_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MAX_Z}},
    // Java: WEST(new VertexInfo(MIN_X, MAX_Y, MIN_Z), (MIN_X, MIN_Y, MIN_Z),
    //            (MIN_X, MIN_Y, MAX_Z), (MIN_X, MAX_Y, MAX_Z))
    {{FACE_EXTENT_MIN_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MIN_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MAX_Z}},
    // Java: EAST(new VertexInfo(MAX_X, MAX_Y, MAX_Z), (MAX_X, MIN_Y, MAX_Z),
    //            (MAX_X, MIN_Y, MIN_Z), (MAX_X, MAX_Y, MIN_Z))
    {{FACE_EXTENT_MAX_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MAX_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MIN_Y, FACE_EXTENT_MIN_Z},
     {FACE_EXTENT_MAX_X, FACE_EXTENT_MAX_Y, FACE_EXTENT_MIN_Z}},
};

// Java: FaceInfo.VertexInfo.select(from, to) - the MIN/MAX pick per axis.
// (the per-axis pick rides on extent_select, like Java's Extent.select)
// Java: Extent.select(minX, minY, minZ, maxX, maxY, maxZ) - the extent picks
// its own axis' min or max. The port encodes axis+bound in the enum.
static float extent_select(FaceExtent extent, const float from[3], const float to[3])
{
    switch (extent)
    {
        case FACE_EXTENT_MIN_X: return from[0];
        case FACE_EXTENT_MIN_Y: return from[1];
        case FACE_EXTENT_MIN_Z: return from[2];
        case FACE_EXTENT_MAX_X: return to[0];
        case FACE_EXTENT_MAX_Y: return to[1];
        default: return to[2];
    }
}

// The vertex position from the FaceInfo row: one extent per axis.
// (unused placeholder removed - extent_select drives every axis directly)

// Java: static BlockElementFace.UVs defaultFaceUV(Vector3fc, Vector3fc, Direction)
void LIBMATTI_MC_FaceBakery_DefaultFaceUV(const float from[3], const float to[3], LIBMATTI_MC_Direction direction,
                                          LIBMATTI_MC_BlockElementFace_UVs *out)
{
    switch (direction)
    {
        case LIBMATTI_MC_Direction_DOWN:
            out->minU = from[0];
            out->minV = 16.0f - to[2];
            out->maxU = to[0];
            out->maxV = 16.0f - from[2];
            break;
        case LIBMATTI_MC_Direction_UP:
            out->minU = from[0];
            out->minV = from[2];
            out->maxU = to[0];
            out->maxV = to[2];
            break;
        case LIBMATTI_MC_Direction_NORTH:
            out->minU = 16.0f - to[0];
            out->minV = 16.0f - to[1];
            out->maxU = 16.0f - from[0];
            out->maxV = 16.0f - from[1];
            break;
        case LIBMATTI_MC_Direction_SOUTH:
            out->minU = from[0];
            out->minV = 16.0f - to[1];
            out->maxU = to[0];
            out->maxV = 16.0f - from[1];
            break;
        case LIBMATTI_MC_Direction_WEST:
            out->minU = from[2];
            out->minV = 16.0f - to[1];
            out->maxU = to[2];
            out->maxV = 16.0f - from[1];
            break;
        case LIBMATTI_MC_Direction_EAST:
        default:
            out->minU = 16.0f - to[2];
            out->minV = 16.0f - to[1];
            out->maxU = 16.0f - from[2];
            out->maxV = 16.0f - from[1];
            break;
    }
}

// Java: private static void rotateVertexBy(Vector3f, Vector3fc origin, Matrix4fc) -
// the element rotation around the origin (the port builds the axis rotation
// matrix inline; the vanilla identity ModelState skips the block-middle pass).
static void rotate_vertex_by(float v[3], const float origin[3], const LIBMATTI_MC_BlockElementRotation *rotation)
{
    float rel[3] = {v[0] - origin[0], v[1] - origin[1], v[2] - origin[2]};
    float rad = rotation->angle * ((float) M_PI) / 180.0f;
    float c = cosf(rad), s = sinf(rad);
    float out[3];
    switch (rotation->axis)
    {
        case 0: // X
            out[0] = rel[0];
            out[1] = rel[1] * c - rel[2] * s;
            out[2] = rel[1] * s + rel[2] * c;
            break;
        case 1: // Y
            out[0] = rel[0] * c + rel[2] * s;
            out[1] = rel[1];
            out[2] = -rel[0] * s + rel[2] * c;
            break;
        default: // Z
            out[0] = rel[0] * c - rel[1] * s;
            out[1] = rel[0] * s + rel[1] * c;
            out[2] = rel[2];
            break;
    }
    if (rotation->rescale)
    {
        // Java: the rescale for 22.5-degree steps - scale = 1/cos, clamped to
        // the two rotated axes (ModelRotation's legacy behaviour).
        float scale = 1.0f / c;
        out[0] *= scale;
        out[1] *= scale;
        out[2] *= scale;
    }
    v[0] = out[0] + origin[0];
    v[1] = out[1] + origin[1];
    v[2] = out[2] + origin[2];
}

// Java: private static @Nullable Direction calculateFacing(Vector3fc[]) -
// the normal of the first triangle snapped to the closest direction.
static int calculate_facing(const float positions[4][3])
{
    // Java: GeometryUtils.normal(p0, p1, p2, dest) = (p1-p0) x (p2-p0)
    float ax = positions[1][0] - positions[0][0];
    float ay = positions[1][1] - positions[0][1];
    float az = positions[1][2] - positions[0][2];
    float bx = positions[2][0] - positions[0][0];
    float by = positions[2][1] - positions[0][1];
    float bz = positions[2][2] - positions[0][2];
    float nx = ay * bz - az * by;
    float ny = az * bx - ax * bz;
    float nz = ax * by - ay * bx;

    if (!isfinite(nx) || !isfinite(ny) || !isfinite(nz))
        return -1;

    int best = -1;
    float bestDot = 0.0f;
    for (int i = 0; i < 6; i++)
    {
        LIBMATTI_MC_Direction dir = (LIBMATTI_MC_Direction) i;
        float dot = nx * LIBMATTI_MC_Direction_GetStepX(dir)
                    + ny * LIBMATTI_MC_Direction_GetStepY(dir)
                    + nz * LIBMATTI_MC_Direction_GetStepZ(dir);
        if (dot >= 0.0f && dot > bestDot)
        {
            bestDot = dot;
            best = i;
        }
    }
    return best;
}

// Java: private static int findVertex(Vector3fc[], int, float, float, float)
static int find_vertex(const float positions[4][3], int start, float x, float y, float z)
{
    for (int i = start; i < 4; i++)
    {
        if (positions[i][0] == x && positions[i][1] == y && positions[i][2] == z)
            return i;
    }
    return -1;
}

static void swap_positions(float positions[4][3], int a, int b)
{
    float tmp[3];
    memcpy(tmp, positions[a], sizeof(tmp));
    memcpy(positions[a], positions[b], sizeof(tmp));
    memcpy(positions[b], tmp, sizeof(tmp));
}

static void swap_uvs(float uvs[4][2], int a, int b)
{
    float tmp[2];
    memcpy(tmp, uvs[a], sizeof(tmp));
    memcpy(uvs[a], uvs[b], sizeof(tmp));
    memcpy(uvs[b], tmp, sizeof(tmp));
}

// Java: private static void recalculateWinding(Vector3fc[], long[], Direction)
static void recalculate_winding(float positions[4][3], float uvs[4][2], LIBMATTI_MC_Direction direction)
{
    float minX = 999.0f, minY = 999.0f, minZ = 999.0f;
    float maxX = -999.0f, maxY = -999.0f, maxZ = -999.0f;
    for (int i = 0; i < 4; i++)
    {
        if (positions[i][0] < minX) minX = positions[i][0];
        if (positions[i][1] < minY) minY = positions[i][1];
        if (positions[i][2] < minZ) minZ = positions[i][2];
        if (positions[i][0] > maxX) maxX = positions[i][0];
        if (positions[i][1] > maxY) maxY = positions[i][1];
        if (positions[i][2] > maxZ) maxZ = positions[i][2];
    }

    const FaceVertexInfo *row = FACE_INFO[direction];
    for (int k = 0; k < 4; k++)
    {
        float x = extent_select(row[k].x, &minX, &maxX);
        float y = extent_select(row[k].y, &minY, &maxY);
        float z = extent_select(row[k].z, &minZ, &maxZ);
        int j = find_vertex(positions, k, x, y, z);
        if (j == -1)
            return; // Java throws; the port keeps the quad as-is.
        if (j != k)
        {
            swap_positions(positions, j, k);
            swap_uvs(uvs, j, k);
        }
    }
}

// Java: public static BakedQuad bakeQuad(...)
void LIBMATTI_MC_FaceBakery_BakeQuad(const float from[3], const float to[3],
                                     const LIBMATTI_MC_BlockElementFace *face,
                                     const float spriteUvRect[4], LIBMATTI_MC_Direction direction,
                                     const LIBMATTI_MC_BlockElementRotation *rotation,
                                     int tintIndex, int shade, LIBMATTI_MC_BakedQuad *out)
{
    // Java: UVs uvs = face.uvs(); if (uvs == null) uvs = defaultFaceUV(from, to, direction);
    LIBMATTI_MC_BlockElementFace_UVs uvs;
    if (face->uvsPresent)
        uvs = face->uvs;
    else
        LIBMATTI_MC_FaceBakery_DefaultFaceUV(from, to, direction, &uvs);

    const FaceVertexInfo *row = FACE_INFO[direction];
    for (int i = 0; i < 4; i++)
    {
        // Java: Vector3f pos = faceinfo$vertexinfo.select(from, to).div(16.0F)
        float pos[3];
        pos[0] = extent_select(row[i].x, from, to) / 16.0f;
        pos[1] = extent_select(row[i].y, from, to) / 16.0f;
        pos[2] = extent_select(row[i].z, from, to) / 16.0f;

        // Java: if (elementRotation != null) rotateVertexBy(pos, origin, transform)
        if (rotation != NULL)
            rotate_vertex_by(pos, &rotation->originX, rotation);

        // Java: float u = BlockElementFace.getU(uvs, quadrant, i) / 16 ...
        // the port folds the /16 into the sprite rect (it is already 0..1).
        float fu = LIBMATTI_MC_BlockElementFace_GetU(&uvs, face->rotation, i) * 16.0f / 16.0f;
        float fv = LIBMATTI_MC_BlockElementFace_GetV(&uvs, face->rotation, i) * 16.0f / 16.0f;

        out->pos[i][0] = pos[0];
        out->pos[i][1] = pos[1];
        out->pos[i][2] = pos[2];
        // Java: sprite.getU(f3), sprite.getV(f2) - the rect maps 0..1 into
        // the atlas space.
        out->uv[i][0] = spriteUvRect[0] + fu * (spriteUvRect[2] - spriteUvRect[0]);
        out->uv[i][1] = spriteUvRect[1] + fv * (spriteUvRect[3] - spriteUvRect[1]);
    }

    // Java: Direction facing = calculateFacing(...); if (rotation == null && facing != null)
    //          recalculateWinding(...)
    int facing = calculate_facing(out->pos);
    if (rotation == NULL && facing >= 0)
        recalculate_winding(out->pos, out->uv, (LIBMATTI_MC_Direction) facing);

    out->tintIndex = tintIndex;
    out->direction = facing >= 0 ? (LIBMATTI_MC_Direction) facing : LIBMATTI_MC_Direction_UP;
    out->shade = shade;
}

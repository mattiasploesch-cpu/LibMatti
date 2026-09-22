// Port of net.minecraft.client.renderer.block.model.BlockElement and
// net.minecraft.client.renderer.block.model.BlockElementRotation.
// The deserializer side rides on the Gson port (Java: BlockElement.Deserializer).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKELEMENT_H
#define MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKELEMENT_H

#include "libmatti/com/google/gson/JsonElement.h"
#include "libmatti/net/minecraft/client/renderer/block/model/BlockElementFace.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public record BlockElementRotation(Vector3fc origin, Axis axis, float angle, boolean rescale)
typedef struct LIBMATTI_MC_BlockElementRotation
{
    // The rotation origin in model space (0..16 coordinates).
    float originX;
    float originY;
    float originZ;
    // Java: Axis - 0 = X, 1 = Y, 2 = Z.
    int axis;
    // Java: the angle in degrees (-45..45, the multiples of 22.5).
    float angle;
    bool rescale;
} LIBMATTI_MC_BlockElementRotation;

// Java: public record BlockElement(Vector3fc from, Vector3fc to,
//           Map<Direction, BlockElementFace> faces, @Nullable BlockElementRotation rotation,
//           boolean shade, int lightEmission)
typedef struct LIBMATTI_MC_BlockElement
{
    // Java: the from/to corners in the 0..16 model space.
    float fromX, fromY, fromZ;
    float toX, toY, toZ;
    // Java: Map<Direction, BlockElementFace> - one face per direction at most.
    // The array is indexed by Direction ordinal; NULL when the face is absent.
    LIBMATTI_MC_BlockElementFace *faces[6];
    LIBMATTI_MC_BlockElementRotation *rotation;
    bool shade;
    int lightEmission;
} LIBMATTI_MC_BlockElement;

// Java: private static Vector3f getPosition(JsonObject, String) - reads the
// three-element coordinate array ("from" / "to"); fails like Java's
// GsonHelper when the member is missing or malformed.
void LIBMATTI_MC_BlockElement_ReadPosition(const LIBMATTI_GSON_JsonElement *object, const char *memberName,
                                           float out[3]);

// Java: private BlockElementRotation getRotation(JsonObject) - NULL when absent
// (caller owns the rotation).
LIBMATTI_MC_BlockElementRotation *LIBMATTI_MC_BlockElement_ReadRotation(const LIBMATTI_GSON_JsonElement *object);

// Java: the Deserializer - one element object into the struct. Faces is filled
// from the "faces" member (each key is the lowercase direction name). Returns
// 0 on malformed input like Java's JsonParseException.
int LIBMATTI_MC_BlockElement_Deserialize(const LIBMATTI_GSON_JsonElement *object, LIBMATTI_MC_BlockElement *out);

// Java: the record is disposed through the owner; the port frees the owned
// faces and rotation.
void LIBMATTI_MC_BlockElement_Free(LIBMATTI_MC_BlockElement *element);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKELEMENT_H

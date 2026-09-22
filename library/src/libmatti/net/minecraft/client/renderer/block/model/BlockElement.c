// Port of net.minecraft.client.renderer.block.model.BlockElement (implementation).

#include "libmatti/net/minecraft/client/renderer/block/model/BlockElement.h"

#include "libmatti/net/minecraft/core/Direction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: Direction.byName(String) - the lowercase face keys of the "faces" object.
static int direction_from_name(const char *name)
{
    int found = 0;
    LIBMATTI_MC_Direction direction = LIBMATTI_MC_Direction_ByName(name, &found);
    return found ? (int) direction : -1;
}

// Java: private static Vector3f getPosition(JsonObject, String)
void LIBMATTI_MC_BlockElement_ReadPosition(const LIBMATTI_GSON_JsonElement *object, const char *memberName,
                                           float out[3])
{
    const LIBMATTI_GSON_JsonElement *array = LIBMATTI_GSON_JsonElement_GetMember(object, memberName);
    if (array == NULL || !LIBMATTI_GSON_JsonElement_IsJsonArray((LIBMATTI_GSON_JsonElement *) array)
        || LIBMATTI_GSON_JsonElement_ElementCount((LIBMATTI_GSON_JsonElement *) array) != 3)
    {
        fprintf(stderr, "ERROR: BlockElement: malformed '%s' coordinates\n", memberName);
        out[0] = out[1] = out[2] = 0.0f;
        return;
    }
    for (int i = 0; i < 3; i++)
    {
        const LIBMATTI_GSON_JsonElement *coord =
            LIBMATTI_GSON_JsonElement_ElementAt((LIBMATTI_GSON_JsonElement *) array, (size_t) i);
        out[i] = coord != NULL ? LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) coord) : 0.0f;
    }
}

// Java: private BlockElementRotation getRotation(JsonObject)
LIBMATTI_MC_BlockElementRotation *LIBMATTI_MC_BlockElement_ReadRotation(const LIBMATTI_GSON_JsonElement *object)
{
    const LIBMATTI_GSON_JsonElement *rotation = LIBMATTI_GSON_JsonElement_GetMember(object, "rotation");
    if (rotation == NULL || LIBMATTI_GSON_JsonElement_IsJsonNull((LIBMATTI_GSON_JsonElement *) rotation))
        return NULL;

    LIBMATTI_MC_BlockElementRotation *out = calloc(1, sizeof(LIBMATTI_MC_BlockElementRotation));
    LIBMATTI_MC_BlockElement_ReadPosition(rotation, "origin", &out->originX);

    const LIBMATTI_GSON_JsonElement *axisElement = LIBMATTI_GSON_JsonElement_GetMember(rotation, "axis");
    if (axisElement != NULL)
    {
        char *axis = LIBMATTI_GSON_JsonElement_GetAsString((LIBMATTI_GSON_JsonElement *) axisElement);
        if (axis != NULL)
        {
            if (strcmp(axis, "x") == 0)
                out->axis = 0;
            else if (strcmp(axis, "y") == 0)
                out->axis = 1;
            else if (strcmp(axis, "z") == 0)
                out->axis = 2;
            free(axis);
        }
    }

    const LIBMATTI_GSON_JsonElement *angleElement = LIBMATTI_GSON_JsonElement_GetMember(rotation, "angle");
    if (angleElement != NULL)
        out->angle = LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) angleElement);

    const LIBMATTI_GSON_JsonElement *rescaleElement = LIBMATTI_GSON_JsonElement_GetMember(rotation, "rescale");
    if (rescaleElement != NULL)
        out->rescale = LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) rescaleElement) != 0.0f;
    return out;
}

// Java: private Map<Direction, BlockElementFace> deserializeFaces(JsonObject) -
// every key maps to the direction by name, the value through the face deserializer.
static int deserialize_faces(const LIBMATTI_GSON_JsonElement *object, LIBMATTI_MC_BlockElement *out)
{
    const LIBMATTI_GSON_JsonElement *faces = LIBMATTI_GSON_JsonElement_GetMember(object, "faces");
    if (faces == NULL || !LIBMATTI_GSON_JsonElement_IsJsonObject((LIBMATTI_GSON_JsonElement *) faces))
    {
        fprintf(stderr, "ERROR: BlockElement: missing 'faces' object\n");
        return 0;
    }
    LIBMATTI_GSON_JsonElement *facesObject = (LIBMATTI_GSON_JsonElement *) faces;
    for (size_t i = 0; i < LIBMATTI_GSON_JsonElement_MemberCount(facesObject); i++)
    {
        const char *name = LIBMATTI_GSON_JsonElement_MemberNameAt(facesObject, i);
        LIBMATTI_GSON_JsonElement *faceJson = LIBMATTI_GSON_JsonElement_MemberValueAt(facesObject, i);
        if (name == NULL || faceJson == NULL)
            continue;
        int direction = direction_from_name(name);
        if (direction < 0)
        {
            fprintf(stderr, "ERROR: BlockElement: unknown face direction '%s'\n", name);
            return 0;
        }

        LIBMATTI_MC_BlockElementFace *face = calloc(1, sizeof(LIBMATTI_MC_BlockElementFace));
        // Java: the face deserializer - tintIndex defaults to -1, rotation to 0.
        face->tintIndex = LIBMATTI_MC_BlockElementFace_NO_TINT;
        const LIBMATTI_GSON_JsonElement *texture = LIBMATTI_GSON_JsonElement_GetMember(faceJson, "texture");
        if (texture != NULL)
        {
            char *ref = LIBMATTI_GSON_JsonElement_GetAsString((LIBMATTI_GSON_JsonElement *) texture);
            face->texture = ref != NULL ? ref : strdup("");
        }
        else
        {
            face->texture = strdup("");
        }
        const LIBMATTI_GSON_JsonElement *tint = LIBMATTI_GSON_JsonElement_GetMember(faceJson, "tintindex");
        if (tint != NULL)
            face->tintIndex = (int) LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) tint);
        const LIBMATTI_GSON_JsonElement *uvElement = LIBMATTI_GSON_JsonElement_GetMember(faceJson, "uv");
        if (uvElement != NULL && LIBMATTI_GSON_JsonElement_IsJsonArray((LIBMATTI_GSON_JsonElement *) uvElement)
            && LIBMATTI_GSON_JsonElement_ElementCount((LIBMATTI_GSON_JsonElement *) uvElement) == 4)
        {
            face->uvsPresent = true;
            for (int c = 0; c < 4; c++)
            {
                float value = LIBMATTI_GSON_JsonElement_GetAsFloat(
                    (LIBMATTI_GSON_JsonElement *) LIBMATTI_GSON_JsonElement_ElementAt(
                        (LIBMATTI_GSON_JsonElement *) uvElement, (size_t) c));
                (&face->uvs.minU)[c] = value;
            }
        }
        const LIBMATTI_GSON_JsonElement *rot = LIBMATTI_GSON_JsonElement_GetMember(faceJson, "rotation");
        if (rot != NULL)
            face->rotation = (int) LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) rot);

        out->faces[direction] = face;
    }
    return 1;
}

// Java: the Deserializer.deserialize chain
int LIBMATTI_MC_BlockElement_Deserialize(const LIBMATTI_GSON_JsonElement *object, LIBMATTI_MC_BlockElement *out)
{
    if (object == NULL || out == NULL)
        return 0;
    memset(out, 0, sizeof(*out));

    LIBMATTI_MC_BlockElement_ReadPosition(object, "from", &out->fromX);
    LIBMATTI_MC_BlockElement_ReadPosition(object, "to", &out->toX);

    // Java: public boolean isShade(...) - the "shade" member defaults to true.
    out->shade = true;
    const LIBMATTI_GSON_JsonElement *shade = LIBMATTI_GSON_JsonElement_GetMember(object, "shade");
    if (shade != NULL)
        out->shade = LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) shade) != 0.0f;

    // Java: this.getLightEmission(jsonobject) - defaults to 0.
    const LIBMATTI_GSON_JsonElement *light = LIBMATTI_GSON_JsonElement_GetMember(object, "light");
    if (light != NULL)
        out->lightEmission = (int) LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) light);

    out->rotation = LIBMATTI_MC_BlockElement_ReadRotation(object);

    if (!deserialize_faces(object, out))
        return 0;
    return 1;
}

// Java: the record is disposed through the owner; the port frees the owned
// faces and rotation.
void LIBMATTI_MC_BlockElement_Free(LIBMATTI_MC_BlockElement *element)
{
    if (element == NULL)
        return;
    for (int i = 0; i < 6; i++)
    {
        if (element->faces[i] != NULL)
        {
            free(element->faces[i]->texture);
            free(element->faces[i]->cullForDirection);
            free(element->faces[i]);
        }
    }
    free(element->rotation);
}

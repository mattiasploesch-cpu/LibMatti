// Port of net.minecraft.client.renderer.block.model.BlockModel (implementation).

#include "libmatti/net/minecraft/client/renderer/block/model/BlockModel.h"

#include "libmatti/com/google/gson/Gson.h"

#include <stdlib.h>
#include <string.h>

// Java: TextureSlots.Data parseTextureMap(JsonObject) - the "textures" member
// into the insertion-ordered name/value pairs.
static void parse_texture_map(const LIBMATTI_GSON_JsonElement *object, LIBMATTI_MC_BlockModel *model)
{
    const LIBMATTI_GSON_JsonElement *textures = LIBMATTI_GSON_JsonElement_GetMember(object, "textures");
    if (textures == NULL || !LIBMATTI_GSON_JsonElement_IsJsonObject((LIBMATTI_GSON_JsonElement *) textures))
        return;
    LIBMATTI_GSON_JsonElement *texturesObject = (LIBMATTI_GSON_JsonElement *) textures;
    size_t count = LIBMATTI_GSON_JsonElement_MemberCount(texturesObject);
    if (count == 0)
        return;

    model->textureCount = count;
    model->textureNames = calloc(count, sizeof(char *));
    model->textureValues = calloc(count, sizeof(char *));
    for (size_t i = 0; i < count; i++)
    {
        const char *name = LIBMATTI_GSON_JsonElement_MemberNameAt(texturesObject, i);
        LIBMATTI_GSON_JsonElement *value = LIBMATTI_GSON_JsonElement_MemberValueAt(texturesObject, i);
        model->textureNames[i] = name != NULL ? strdup(name) : strdup("");
        char *resolved = value != NULL
                             ? LIBMATTI_GSON_JsonElement_GetAsString((LIBMATTI_GSON_JsonElement *) value)
                             : NULL;
        model->textureValues[i] = resolved != NULL ? resolved : strdup("");
    }
}

// Java: the Deserializer - "elements" may be absent (inherit-only models).
static void parse_elements(const LIBMATTI_GSON_JsonElement *object, LIBMATTI_MC_BlockModel *model)
{
    const LIBMATTI_GSON_JsonElement *elements = LIBMATTI_GSON_JsonElement_GetMember(object, "elements");
    if (elements == NULL || !LIBMATTI_GSON_JsonElement_IsJsonArray((LIBMATTI_GSON_JsonElement *) elements))
        return; // Java: getElements() returns null - the model inherits.

    LIBMATTI_GSON_JsonElement *array = (LIBMATTI_GSON_JsonElement *) elements;
    size_t count = LIBMATTI_GSON_JsonElement_ElementCount(array);
    model->elementsPresent = 1;
    model->elementCount = count;
    if (count == 0)
        return;
    model->elements = calloc(count, sizeof(LIBMATTI_MC_BlockElement));
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_GSON_JsonElement *elementJson = LIBMATTI_GSON_JsonElement_ElementAt(array, i);
        if (elementJson == NULL)
            continue;
        // Java: a malformed element throws; the port skips the element by
        // leaving the struct zeroed (the free side checks it).
        LIBMATTI_MC_BlockElement_Deserialize(elementJson, &model->elements[i]);
    }
}

// Java: public static BlockModel fromString(String) / the Deserializer
LIBMATTI_MC_BlockModel *LIBMATTI_MC_BlockModel_Parse(const char *jsonText)
{
    if (jsonText == NULL)
        return NULL;

    LIBMATTI_GSON_Gson *gson = LIBMATTI_GSON_Gson_New();
    if (gson == NULL)
        return NULL;
    LIBMATTI_GSON_JsonElement *root = LIBMATTI_GSON_Gson_FromJson(gson, jsonText, strlen(jsonText));
    LIBMATTI_GSON_Gson_Free(gson);
    if (root == NULL || !LIBMATTI_GSON_JsonElement_IsJsonObject(root))
    {
        // Java: JsonParseException
        return NULL;
    }

    LIBMATTI_MC_BlockModel *model = calloc(1, sizeof(LIBMATTI_MC_BlockModel));

    // Java: this.name = GsonHelper.getAsString(jsonobject, "parent", "") -
    // an empty string means "no parent" on the port side (NULL).
    const LIBMATTI_GSON_JsonElement *parent = LIBMATTI_GSON_JsonElement_GetMember(root, "parent");
    if (parent != NULL && !LIBMATTI_GSON_JsonElement_IsJsonNull(parent))
    {
        char *name = LIBMATTI_GSON_JsonElement_GetAsString((LIBMATTI_GSON_JsonElement *) parent);
        model->parent = (name != NULL && name[0] != '\0') ? name : NULL;
        if (model->parent == NULL)
            free(name);
    }

    // Java: boolean hasAmbientOcclusion = GsonHelper.getAsBoolean(jsonobject, "ambientocclusion", true);
    model->ambientOcclusion = 1;
    const LIBMATTI_GSON_JsonElement *ao = LIBMATTI_GSON_JsonElement_GetMember(root, "ambientocclusion");
    if (ao != NULL)
        model->ambientOcclusion = LIBMATTI_GSON_JsonElement_GetAsFloat((LIBMATTI_GSON_JsonElement *) ao) != 0.0f;

    parse_texture_map(root, model);
    parse_elements(root, model);
    return model;
}

// Java: TextureSlots.Resolver.resolve(String) - the "#ref" chase through the
// model's own map (the parent chain rides on the baker walking parents first).
char *LIBMATTI_MC_BlockModel_ResolveTexture(const LIBMATTI_MC_BlockModel *model, const char *slotName)
{
    if (model == NULL || slotName == NULL)
        return NULL;
    // Java: the slot name starts with '#' in the face reference ("#all").
    const char *key = slotName[0] == '#' ? slotName + 1 : slotName;
    for (int depth = 0; depth < 10; depth++) // Java's reference loop guard
    {
        const char *value = NULL;
        for (size_t i = 0; i < model->textureCount; i++)
        {
            if (strcmp(model->textureNames[i], key) == 0)
            {
                value = model->textureValues[i];
                break;
            }
        }
        if (value == NULL)
            return NULL; // Java: the resolver chain fails - missing slot.
        if (value[0] != '#')
            return strdup(value); // a real texture id.
        key = value + 1; // another reference - keep chasing.
    }
    return NULL; // Java: reference loop - give up.
}

const char *LIBMATTI_MC_BlockModel_Parent(const LIBMATTI_MC_BlockModel *model)
{
    return model != NULL ? model->parent : NULL;
}

void LIBMATTI_MC_BlockModel_Free(LIBMATTI_MC_BlockModel *model)
{
    if (model == NULL)
        return;
    free(model->parent);
    for (size_t i = 0; i < model->textureCount; i++)
    {
        free(model->textureNames[i]);
        free(model->textureValues[i]);
    }
    free(model->textureNames);
    free(model->textureValues);
    if (model->elements != NULL)
    {
        for (size_t i = 0; i < model->elementCount; i++)
        {
            // Java: the record is owned; only free elements the deserializer
            // actually filled (a failed parse leaves an all-zero struct).
            if (model->elements[i].faces[0] != NULL || model->elements[i].faces[5] != NULL
                || model->elements[i].fromX != 0.0f || model->elements[i].toX != 0.0f)
            {
                LIBMATTI_MC_BlockElement_Free(&model->elements[i]);
            }
        }
        free(model->elements);
    }
    free(model);
}

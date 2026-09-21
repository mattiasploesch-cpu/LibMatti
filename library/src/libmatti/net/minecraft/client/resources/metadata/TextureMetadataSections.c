// Implementation of the texture/animation metadata section parsers. Java runs
// the DataResult codecs; the port walks the parsed Gson JSON object with the
// same field names and defaults.

#include "libmatti/net/minecraft/client/resources/metadata/TextureMetadataSections.h"

#include "libmatti/com/google/gson/JsonElement.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// MipmapStrategy
// ---------------------------------------------------------------------------

static const char *const MIPMAP_STRATEGY_NAMES[] = {
    "auto", "mean", "cutout", "strict_cutout", "dark_cutout",
};

// Java: the StringRepresentable codec of the MipmapStrategy enum
int LIBMATTI_MC_MipmapStrategy_FromName(const char *name, int *outStrategy)
{
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(MIPMAP_STRATEGY_NAMES[i], name) == 0)
        {
            *outStrategy = i;
            return 1;
        }
    }
    return 0;
}

const char *LIBMATTI_MC_MipmapStrategy_ToName(int strategy)
{
    if (strategy < 0 || strategy >= 5)
        return "auto";
    return MIPMAP_STRATEGY_NAMES[strategy];
}

// ---------------------------------------------------------------------------
// TextureMetadataSection
// ---------------------------------------------------------------------------

LIBMATTI_MC_TextureMetadataSection *LIBMATTI_MC_TextureMetadataSection_Parse(const void *jsonObject, char **outError)
{
    const LIBMATTI_GSON_JsonElement *object = (const LIBMATTI_GSON_JsonElement *) jsonObject;
    if (object == NULL || !LIBMATTI_GSON_JsonElement_IsJsonObject(object))
        return NULL;

    LIBMATTI_MC_TextureMetadataSection *section = calloc(1, sizeof(LIBMATTI_MC_TextureMetadataSection));
    // Java: optionalFieldOf defaults
    section->blur = 0;
    section->clamp = 0;
    section->mipmapStrategy = 0;
    section->alphaCutoffBias = 0.0f;

    LIBMATTI_GSON_JsonElement *member = LIBMATTI_GSON_JsonElement_GetMember(object, "blur");
    if (member != NULL)
        section->blur = LIBMATTI_GSON_JsonElement_GetAsBoolean(member) ? 1 : 0;

    member = LIBMATTI_GSON_JsonElement_GetMember(object, "clamp");
    if (member != NULL)
        section->clamp = LIBMATTI_GSON_JsonElement_GetAsBoolean(member) ? 1 : 0;

    member = LIBMATTI_GSON_JsonElement_GetMember(object, "mipmap_strategy");
    if (member != NULL)
    {
        char *name = LIBMATTI_GSON_JsonElement_GetAsString(member);
        if (name == NULL || !LIBMATTI_MC_MipmapStrategy_FromName(name, &section->mipmapStrategy))
        {
            free(name);
            free(section);
            if (outError != NULL)
                *outError = strdup("Invalid mipmap_strategy");
            return NULL;
        }
        free(name);
    }

    member = LIBMATTI_GSON_JsonElement_GetMember(object, "alpha_cutoff_bias");
    if (member != NULL)
        section->alphaCutoffBias = (float) LIBMATTI_GSON_JsonElement_GetAsDouble(member);

    return section;
}

void LIBMATTI_MC_TextureMetadataSection_Free(LIBMATTI_MC_TextureMetadataSection *section)
{
    free(section);
}

// ---------------------------------------------------------------------------
// AnimationMetadataSection
// ---------------------------------------------------------------------------

LIBMATTI_MC_FrameSize LIBMATTI_MC_AnimationMetadataSection_CalculateFrameSize(
    const LIBMATTI_MC_AnimationMetadataSection *section, int imageWidth, int imageHeight)
{
    // Java: public FrameSize calculateFrameSize(int, int)
    LIBMATTI_MC_FrameSize frameSize;
    if (section->hasFrameWidth)
    {
        frameSize.width = section->frameWidth;
        frameSize.height = section->hasFrameHeight ? section->frameHeight : imageHeight;
        if (section->hasFrameHeight)
        {
            frameSize.width = section->frameWidth;
            frameSize.height = section->frameHeight;
        }
        else
        {
            frameSize.height = imageHeight;
        }
    }
    else if (section->hasFrameHeight)
    {
        frameSize.width = imageWidth;
        frameSize.height = section->frameHeight;
    }
    else
    {
        int min = imageWidth < imageHeight ? imageWidth : imageHeight;
        frameSize.width = min;
        frameSize.height = min;
    }
    return frameSize;
}

// Java: AnimationFrame.CODEC - index (or the array position) + optional frametime
static void parse_frames(const LIBMATTI_GSON_JsonElement *array, LIBMATTI_MC_AnimationMetadataSection *section)
{
    size_t count = LIBMATTI_GSON_JsonElement_ElementCount(array);
    section->hasFrames = 1;
    section->frameCount = count;
    section->frames = calloc(count > 0 ? count : 1, sizeof(LIBMATTI_MC_AnimationFrame));

    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_GSON_JsonElement *element = LIBMATTI_GSON_JsonElement_ElementAt(array, i);
        if (LIBMATTI_GSON_JsonElement_IsJsonObject(element))
        {
            // Java: AnimationFrame.MapCodec - index is required, frametime defaults
            LIBMATTI_GSON_JsonElement *index = LIBMATTI_GSON_JsonElement_GetMember(element, "index");
            LIBMATTI_GSON_JsonElement *time = LIBMATTI_GSON_JsonElement_GetMember(element, "frametime");
            section->frames[i].index = index != NULL ? LIBMATTI_GSON_JsonElement_GetAsInt(index) : (int) i;
            section->frames[i].time = time != NULL ? LIBMATTI_GSON_JsonElement_GetAsInt(time) : -1;
        }
        else
        {
            // Java: ExtraCodecs.POSITIVE_INT - the plain index form
            section->frames[i].index = LIBMATTI_GSON_JsonElement_GetAsInt(element);
            section->frames[i].time = -1; // resolved to the default later
        }
    }
}

LIBMATTI_MC_AnimationMetadataSection *LIBMATTI_MC_AnimationMetadataSection_Parse(const void *jsonObject, char **outError)
{
    const LIBMATTI_GSON_JsonElement *object = (const LIBMATTI_GSON_JsonElement *) jsonObject;
    if (object == NULL || !LIBMATTI_GSON_JsonElement_IsJsonObject(object))
        return NULL;

    LIBMATTI_MC_AnimationMetadataSection *section = calloc(1, sizeof(LIBMATTI_MC_AnimationMetadataSection));
    // Java: the codec defaults - frametime 1, interpolate false
    section->defaultFrameTime = 1;
    section->interpolatedFrames = 0;

    LIBMATTI_GSON_JsonElement *member = LIBMATTI_GSON_JsonElement_GetMember(object, "frames");
    if (member != NULL && LIBMATTI_GSON_JsonElement_IsJsonArray(member))
        parse_frames(member, section);

    member = LIBMATTI_GSON_JsonElement_GetMember(object, "width");
    if (member != NULL)
    {
        section->hasFrameWidth = 1;
        section->frameWidth = LIBMATTI_GSON_JsonElement_GetAsInt(member);
    }

    member = LIBMATTI_GSON_JsonElement_GetMember(object, "height");
    if (member != NULL)
    {
        section->hasFrameHeight = 1;
        section->frameHeight = LIBMATTI_GSON_JsonElement_GetAsInt(member);
    }

    member = LIBMATTI_GSON_JsonElement_GetMember(object, "frametime");
    if (member != NULL)
        section->defaultFrameTime = LIBMATTI_GSON_JsonElement_GetAsInt(member);

    member = LIBMATTI_GSON_JsonElement_GetMember(object, "interpolate");
    if (member != NULL)
        section->interpolatedFrames = LIBMATTI_GSON_JsonElement_GetAsBoolean(member) ? 1 : 0;

    return section;
}

void LIBMATTI_MC_AnimationMetadataSection_Free(LIBMATTI_MC_AnimationMetadataSection *section)
{
    if (section == NULL)
        return;
    free(section->frames);
    free(section);
}

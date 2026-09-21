// Port of net.minecraft.server.packs.metadata.pack.PackFormat.

#include "libmatti/net/minecraft/server/packs/repository/PackFormat.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_PackFormat LIBMATTI_MC_PackFormat_Of(int major, int minor)
{
    LIBMATTI_MC_PackFormat format = {major, minor};
    return format;
}

LIBMATTI_MC_PackFormat LIBMATTI_MC_PackFormat_OfSingle(int major)
{
    return LIBMATTI_MC_PackFormat_Of(major, 0);
}

int LIBMATTI_MC_PackFormat_CompareTo(const LIBMATTI_MC_PackFormat *a, const LIBMATTI_MC_PackFormat *b)
{
    if (a->major != b->major) return a->major < b->major ? -1 : 1;
    if (a->minor != b->minor) return a->minor < b->minor ? -1 : 1;
    return 0;
}

LIBMATTI_MC_InclusiveRange LIBMATTI_MC_PackFormat_MinorRange(const LIBMATTI_MC_PackFormat *format)
{
    return LIBMATTI_MC_InclusiveRange_Of(format->major, INT_MAX);
}

int LIBMATTI_MC_PackFormat_LastPreMinorVersion(int packType)
{
    switch (packType)
    {
        case 0 /* CLIENT_RESOURCES */: return 64;
        case 1 /* SERVER_DATA */: return 81;
    }
    return 64;
}

int LIBMATTI_MC_PackFormat_IntermediaryFormat_EffectiveMinMajorVersion(
    const LIBMATTI_MC_PackFormat_IntermediaryFormat *holder)
{
    // Java: min.isPresent() ? (supported.isPresent() ? min(min.major, supported.min) : min.major)
    //                     : (supported.isPresent() ? supported.min : Integer.MAX_VALUE)
    if (holder->hasMin)
    {
        if (holder->hasSupported)
            return holder->min.major < holder->supported.minInclusive ? holder->min.major
                                                                      : holder->supported.minInclusive;
        return holder->min.major;
    }
    if (holder->hasSupported) return holder->supported.minInclusive;
    return INT_MAX;
}

// Java: private String validatePackFormatForRange(int min, int max)
static char *validate_pack_format_for_range(const LIBMATTI_MC_PackFormat_IntermediaryFormat *holder, int min, int max)
{
    int format = holder->format;
    if (format < min || format > max)
    {
        char *message = malloc(160);
        snprintf(message, 160, "Pack declared support for versions %d to %d but declared main format is %d", min, max,
                 format);
        return message;
    }
    if (format < 15)
    {
        char *message = malloc(160);
        snprintf(message, 160,
                 "Multi-version packs cannot support minimum version of less than 15, since this will leave versions "
                 "in range unable to load pack.");
        return message;
    }
    return NULL;
}

int LIBMATTI_MC_PackFormat_IntermediaryFormat_Validate(const LIBMATTI_MC_PackFormat_IntermediaryFormat *holder,
                                                       int lastPreMinorVersion, int isPack, int isOverlay,
                                                       const char *name, const char *key,
                                                       LIBMATTI_MC_InclusiveRange *outRange, char **outError)
{
    *outError = NULL;

    // Java: if (min.isPresent() != max.isPresent()) -> error "missing field"
    if (holder->hasMin != holder->hasMax)
    {
        size_t length = strlen(name) + 64;
        char *message = malloc(length);
        snprintf(message, length, "%s missing field, must declare both min_format and max_format", name);
        *outError = message;
        return 0;
    }
    // Java: else if (isOverlay && supported.isEmpty()) -> error "missing required field"
    if (isOverlay && !holder->hasSupported)
    {
        size_t length = strlen(name) + strlen(key) + 128;
        char *message = malloc(length);
        snprintf(message, length,
                 "%s missing required field %s, must be present in all overlays for any overlays to work across game "
                 "versions",
                 name, key);
        *outError = message;
        return 0;
    }
    // Java: else if (min.isPresent()) -> validateNewFormat
    if (holder->hasMin)
    {
        int min = holder->min.major;
        int max = holder->max.major;
        if (LIBMATTI_MC_PackFormat_CompareTo(&holder->min, &holder->max) > 0)
        {
            size_t length = strlen(name) + 96;
            char *message = malloc(length);
            snprintf(message, length, "%s min_format (%d.%d) is greater than max_format (%d.%d)", name, holder->min.major,
                     holder->min.minor, holder->max.major, holder->max.minor);
            *outError = message;
            return 0;
        }
        if (min > lastPreMinorVersion && !isOverlay)
        {
            if (holder->hasSupported)
            {
                size_t length = strlen(name) + strlen(key) + 160;
                char *message = malloc(length);
                snprintf(message, length,
                         "%s key %s is deprecated starting from pack format %d. Remove %s from your pack.mcmeta.", name,
                         key, lastPreMinorVersion + 1, key);
                *outError = message;
                return 0;
            }
            if (isPack && holder->hasFormat)
            {
                char *message = validate_pack_format_for_range(holder, min, max);
                if (message != NULL)
                {
                    *outError = message;
                    return 0;
                }
            }
        }
        else
        {
            if (!holder->hasSupported)
            {
                size_t length = strlen(name) + strlen(key) + 256;
                char *message = malloc(length);
                snprintf(message, length,
                         "%s declares support for format %d, but game versions supporting formats 17 to %d require a "
                         "%s field. Add \"%s\": [%d, %d] or require a version greater or equal to %d.0.",
                         name, min, lastPreMinorVersion, key, key, min, lastPreMinorVersion, lastPreMinorVersion + 1);
                *outError = message;
                return 0;
            }
            if (holder->supported.minInclusive != min)
            {
                size_t length = strlen(name) + strlen(key) + 160;
                char *message = malloc(length);
                snprintf(message, length,
                         "%s version declaration mismatch between %s (from %d) and min_format (%d.%d)", name, key,
                         holder->supported.minInclusive, holder->min.major, holder->min.minor);
                *outError = message;
                return 0;
            }
            if (holder->supported.maxInclusive != max && holder->supported.maxInclusive != lastPreMinorVersion)
            {
                size_t length = strlen(name) + strlen(key) + 160;
                char *message = malloc(length);
                snprintf(message, length,
                         "%s version declaration mismatch between %s (up to %d) and max_format (%d.%d)", name, key,
                         holder->supported.maxInclusive, holder->max.major, holder->max.minor);
                *outError = message;
                return 0;
            }
            if (isPack)
            {
                if (!holder->hasFormat)
                {
                    size_t length = strlen(name) + strlen(key) + 256;
                    char *message = malloc(length);
                    snprintf(message, length,
                             "%s declares support for formats up to %d, but game versions supporting formats 17 to %d "
                             "require a pack_format field. Add \"pack_format\": %d or require a version greater or "
                             "equal to %d.0.",
                             name, lastPreMinorVersion, lastPreMinorVersion, min, lastPreMinorVersion + 1);
                    *outError = message;
                    return 0;
                }
                char *message = validate_pack_format_for_range(holder, min, max);
                if (message != NULL)
                {
                    *outError = message;
                    return 0;
                }
            }
        }
        *outRange = LIBMATTI_MC_InclusiveRange_Of(holder->min.major, holder->max.major);
        return 1;
    }
    // Java: else if (supported.isPresent()) -> validateOldFormat
    if (holder->hasSupported)
    {
        int min = holder->supported.minInclusive;
        int max = holder->supported.maxInclusive;
        if (max > lastPreMinorVersion)
        {
            size_t length = strlen(name) + 128;
            char *message = malloc(length);
            snprintf(message, length,
                     "%s declares support for version newer than %d, but is missing mandatory fields min_format and "
                     "max_format",
                     name, lastPreMinorVersion);
            *outError = message;
            return 0;
        }
        if (isPack)
        {
            if (!holder->hasFormat)
            {
                size_t length = strlen(name) + strlen(key) + 256;
                char *message = malloc(length);
                snprintf(message, length,
                         "%s declares support for formats up to %d, but game versions supporting formats 17 to %d "
                         "require a pack_format field. Add \"pack_format\": %d or require a version greater or equal "
                         "to %d.0.",
                         name, lastPreMinorVersion, lastPreMinorVersion, min, lastPreMinorVersion + 1);
                *outError = message;
                return 0;
            }
            char *message = validate_pack_format_for_range(holder, min, max);
            if (message != NULL)
            {
                *outError = message;
                return 0;
            }
        }
        *outRange = LIBMATTI_MC_InclusiveRange_Of(min, max);
        return 1;
    }
    // Java: else if (isPack && format.isPresent())
    if (isPack && holder->hasFormat)
    {
        int format = holder->format;
        if (format > lastPreMinorVersion)
        {
            size_t length = strlen(name) + 160;
            char *message = malloc(length);
            snprintf(message, length,
                     "%s declares support for version newer than %d, but is missing mandatory fields min_format and "
                     "max_format",
                     name, lastPreMinorVersion);
            *outError = message;
            return 0;
        }
        *outRange = LIBMATTI_MC_InclusiveRange_OfSingle(format);
        return 1;
    }
    // Java: else -> error "could not be parsed"
    {
        size_t length = strlen(name) + 96;
        char *message = malloc(length);
        snprintf(message, length, "%s could not be parsed, missing format version information", name);
        *outError = message;
        return 0;
    }
}

LIBMATTI_MC_PackFormat_IntermediaryFormat LIBMATTI_MC_PackFormat_IntermediaryFormat_FromRange(
    const LIBMATTI_MC_InclusiveRange *range, int lastPreMinorVersion)
{
    LIBMATTI_MC_PackFormat_IntermediaryFormat holder;
    memset(&holder, 0, sizeof(holder));
    holder.hasMin = 1;
    holder.min = LIBMATTI_MC_PackFormat_Of(range->minInclusive, 0);
    holder.hasMax = 1;
    holder.max = LIBMATTI_MC_PackFormat_Of(range->maxInclusive, 0);
    // Java: inclusiverange = range.map(PackFormat::major); format/supported present only when
    // the lastPreMinorVersion sits inside the range
    if (LIBMATTI_MC_InclusiveRange_IsValueInRange(range, lastPreMinorVersion))
    {
        holder.hasFormat = 1;
        holder.format = range->minInclusive;
        holder.hasSupported = 1;
        holder.supported = LIBMATTI_MC_InclusiveRange_Of(range->minInclusive, range->maxInclusive);
    }
    return holder;
}

int LIBMATTI_MC_PackFormat_ValidateHolderList(const LIBMATTI_MC_PackFormat_IntermediaryFormat *formats,
                                              const char *const *overlays, size_t count, int lastPreMinorVersion,
                                              LIBMATTI_MC_InclusiveRange *outRanges, char **outError)
{
    *outError = NULL;

    // Java: i = min of effectiveMinMajorVersion over the holders, orElse MAX_VALUE
    int minEffective = INT_MAX;
    for (size_t i = 0; i < count; i++)
    {
        int effective = LIBMATTI_MC_PackFormat_IntermediaryFormat_EffectiveMinMajorVersion(&formats[i]);
        if (effective < minEffective) minEffective = effective;
    }

    for (size_t i = 0; i < count; i++)
    {
        const LIBMATTI_MC_PackFormat_IntermediaryFormat *holder = &formats[i];
        // Java: warn "Unknown or broken overlay entry" for a fully empty holder
        if (!holder->hasMin && !holder->hasMax && !holder->hasSupported)
        {
            char *message = malloc(160);
            snprintf(message, 160, "Unknown or broken overlay entry %s", overlays[i]);
            *outError = message;
            return 0;
        }
        LIBMATTI_MC_InclusiveRange range;
        char *error = NULL;
        // Java: validate(i <= p_425651_, ...) - the effective-min check relaxes the deprecated key
        if (!LIBMATTI_MC_PackFormat_IntermediaryFormat_Validate(holder, lastPreMinorVersion, 0,
                                                                minEffective <= lastPreMinorVersion, overlays[i],
                                                                "formats", &range, &error))
        {
            *outError = error;
            return 0;
        }
        outRanges[i] = range;
    }
    return 1;
}

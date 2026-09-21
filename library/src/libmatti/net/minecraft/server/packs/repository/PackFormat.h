// Port of net.minecraft.server.packs.metadata.pack.PackFormat.
// The record and the IntermediaryFormat validate logic are ported 1:1; the Codec
// plumbing becomes the JSON reader in PackMetadataSection.

#ifndef MATTICRAFT_MC_PACKS_REPOSITORY_PACKFORMAT_H
#define MATTICRAFT_MC_PACKS_REPOSITORY_PACKFORMAT_H

#include <stddef.h>

#include "libmatti/net/minecraft/util/InclusiveRange.h"

// Java: public record PackFormat(int major, int minor) implements Comparable<PackFormat>
typedef struct
{
    int major;
    int minor;
} LIBMATTI_MC_PackFormat;

// Java: public static PackFormat of(int major, int minor) / of(int single)
LIBMATTI_MC_PackFormat LIBMATTI_MC_PackFormat_Of(int major, int minor);
LIBMATTI_MC_PackFormat LIBMATTI_MC_PackFormat_OfSingle(int major);

// Java: public int compareTo(PackFormat)
int LIBMATTI_MC_PackFormat_CompareTo(const LIBMATTI_MC_PackFormat *a, const LIBMATTI_MC_PackFormat *b);
// Java: public InclusiveRange<PackFormat> minorRange()
LIBMATTI_MC_InclusiveRange LIBMATTI_MC_PackFormat_MinorRange(const LIBMATTI_MC_PackFormat *format);
// Java: public static int lastPreMinorVersion(PackType)
int LIBMATTI_MC_PackFormat_LastPreMinorVersion(int packType);

// Java: record IntermediaryFormat(Optional<PackFormat> min, Optional<PackFormat> max,
//                                 Optional<Integer> format, Optional<InclusiveRange<Integer>> supported)
typedef struct
{
    int hasMin;
    LIBMATTI_MC_PackFormat min;
    int hasMax;
    LIBMATTI_MC_PackFormat max;
    int hasFormat;
    int format;
    int hasSupported;
    LIBMATTI_MC_InclusiveRange supported;
} LIBMATTI_MC_PackFormat_IntermediaryFormat;

// Java: public int effectiveMinMajorVersion()
int LIBMATTI_MC_PackFormat_IntermediaryFormat_EffectiveMinMajorVersion(
    const LIBMATTI_MC_PackFormat_IntermediaryFormat *holder);
// Java: public DataResult<InclusiveRange<PackFormat>> validate(int lastPreMinorVersion, boolean isPack,
//         boolean isOverlay, String name, String key)
// Returns 1 with *outRange set on success, 0 with *outError (caller frees) on failure.
int LIBMATTI_MC_PackFormat_IntermediaryFormat_Validate(const LIBMATTI_MC_PackFormat_IntermediaryFormat *holder,
                                                       int lastPreMinorVersion, int isPack, int isOverlay,
                                                       const char *name, const char *key,
                                                       LIBMATTI_MC_InclusiveRange *outRange, char **outError);
// Java: static IntermediaryFormat fromRange(InclusiveRange<PackFormat>, int lastPreMinorVersion)
LIBMATTI_MC_PackFormat_IntermediaryFormat LIBMATTI_MC_PackFormat_IntermediaryFormat_FromRange(
    const LIBMATTI_MC_InclusiveRange *range, int lastPreMinorVersion);
// Java: validateHolderList - validates an overlay entry list; returns the applicable count,
// filling outRanges/outOverlays in parallel; 0 with *outError (caller frees) on failure
int LIBMATTI_MC_PackFormat_ValidateHolderList(const LIBMATTI_MC_PackFormat_IntermediaryFormat *formats,
                                              const char *const *overlays, size_t count, int lastPreMinorVersion,
                                              LIBMATTI_MC_InclusiveRange *outRanges, char **outError);

#endif //MATTICRAFT_MC_PACKS_REPOSITORY_PACKFORMAT_H

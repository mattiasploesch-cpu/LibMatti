// Port of net.minecraft.server.packs.PackLocationInfo and
// net.minecraft.server.packs.repository.PackSource.

#include "libmatti/net/minecraft/server/packs/PackLocationInfo.h"

#include <stdlib.h>
#include <string.h>

static const LIBMATTI_MC_PackSource NONE = {LIBMATTI_MC_PACK_SOURCE_NONE, NULL, 0, 0};
static const LIBMATTI_MC_PackSource BUILT_IN = {LIBMATTI_MC_PACK_SOURCE_BUILT_IN, "pack.source.builtin", 1, 0};
static const LIBMATTI_MC_PackSource VANILLA = {LIBMATTI_MC_PACK_SOURCE_VANILLA, "pack.source.vanilla", 0, 0};
static const LIBMATTI_MC_PackSource WORLD = {LIBMATTI_MC_PACK_SOURCE_WORLD, "pack.source.world", 0, 0};
static const LIBMATTI_MC_PackSource SERVER = {LIBMATTI_MC_PACK_SOURCE_SERVER, "pack.source.server", 0, 1};
static const LIBMATTI_MC_PackSource DEFAULT = {LIBMATTI_MC_PACK_SOURCE_DEFAULT, "pack.source.default", 1, 0};
static const LIBMATTI_MC_PackSource BUILT_IN_OLD = {LIBMATTI_MC_PACK_SOURCE_BUILT_IN_OLD, "pack.source.builtinOld", 1, 0};
static const LIBMATTI_MC_PackSource FEATURE = {LIBMATTI_MC_PACK_SOURCE_FEATURE, "pack.source.feature", 1, 0};
static const LIBMATTI_MC_PackSource TEST = {LIBMATTI_MC_PACK_SOURCE_TEST, "pack.source.test", 1, 0};
static const LIBMATTI_MC_PackSource UNKNOWN_PACK = {LIBMATTI_MC_PACK_SOURCE_UNKNOWN_PACK, "pack.source.unknown", 0, 0};

const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_None(void) { return &NONE; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_BuiltIn(void) { return &BUILT_IN; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Vanilla(void) { return &VANILLA; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_World(void) { return &WORLD; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Server(void) { return &SERVER; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Default(void) { return &DEFAULT; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_BuiltInOld(void) { return &BUILT_IN_OLD; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Feature(void) { return &FEATURE; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_Test(void) { return &TEST; }
const LIBMATTI_MC_PackSource *LIBMATTI_MC_PackSource_UnknownPack(void) { return &UNKNOWN_PACK; }

LIBMATTI_MC_PackLocationInfo *LIBMATTI_MC_PackLocationInfo_New(const char *id, const char *title,
                                                               const LIBMATTI_MC_PackSource *source)
{
    LIBMATTI_MC_PackLocationInfo *info = calloc(1, sizeof(LIBMATTI_MC_PackLocationInfo));
    info->id = strdup(id);
    info->title = strdup(title);
    info->source = source;
    return info;
}

void LIBMATTI_MC_PackLocationInfo_Free(LIBMATTI_MC_PackLocationInfo *info)
{
    free(info->id);
    free(info->title);
    free(info->knownPackNamespace);
    free(info->knownPackId);
    free(info->knownPackVersion);
    free(info);
}

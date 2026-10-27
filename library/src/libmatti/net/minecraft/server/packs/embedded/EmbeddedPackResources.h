// Port of net.minecraft.server.packs.VanillaPackResources (the built-in pack)
// over the gzip blob the build links into the executable. Java's vanilla pack
// rides the classpath; the port rides the .a: the build tool (tools/
// embed_resources.c) packs library/resources/ into a deterministic tar + gzip
// blob, the generated EmbeddedPackResources_data.c carries it as a C array and
// the runtime inflates it lazily into RAM on the first access - the pack reads
// only from the process image, nothing touches the filesystem (no --assetsRoot,
// no pack dir on disk, the blob ships inside the executable).
//
// The blob's tar layout mirrors the vanilla pack root ("assets/", "data/"), so
// open()/listResources()/getNamespaces() answer exactly like Java's pack: the
// members live under "<assets|data>/<namespace>/...", the root resources
// ("pack.mcmeta") under "./".

#ifndef MATTICRAFT_MC_PACKS_EMBEDDED_EMBEDDEDPACKRESOURCES_H
#define MATTICRAFT_MC_PACKS_EMBEDDED_EMBEDDEDPACKRESOURCES_H

#include "libmatti/net/minecraft/server/packs/PackResources.h"

#include <stddef.h>

// Java: VanillaPackResourcesBuilder - the built-in pack over the embedded
// blob. The whole tar index + the inflated bytes live in RAM (the process
// image owns the blob; the inflate output is malloc'd once and shared).
typedef struct LIBMATTI_MC_EmbeddedPackResources
{
    const LIBMATTI_MC_PackResources_VTable *vtable;
    LIBMATTI_MC_PackLocationInfo *location;

    // The inflated tar (lazy, on the first access - a 60 MB page the pack
    // reads straight from RAM; nothing is written to disk).
    unsigned char *tarData;
    size_t tarSize;
    int inflated;

    // The tar index: the file members sorted by name (the listResources
    // prefix walk bisects over them).
    char **names;       // the member names ("./assets/matticraft/...")
    size_t *offsets;    // the header offset in the tar
    size_t *sizes;      // the member payload size
    size_t entryCount;

    // Java: the rootResources map (the metadata the pack carries: pack.mcmeta
    // et al. - the generated header exposes the blob, this struct indexes it).
    unsigned char *packMcmeta;
    size_t packMcmetaSize;
} LIBMATTI_MC_EmbeddedPackResources;

// Java: new VanillaPackResources(builder) - the pack over the linked-in blob.
// The bundle path is the blob (never opened from disk - the executable's data
// section carries it). Returns NULL when the blob is missing.
LIBMATTI_MC_EmbeddedPackResources *LIBMATTI_MC_EmbeddedPackResources_New(void);
LIBMATTI_MC_EmbeddedPackResources *LIBMATTI_MC_EmbeddedPackResources_NewWithLocation(
    LIBMATTI_MC_PackLocationInfo *location);
void LIBMATTI_MC_EmbeddedPackResources_Free(LIBMATTI_MC_EmbeddedPackResources *pack);

// Java: VanillaPackResources.readResource - the raw bytes of the member
// ("./assets/matticraft/textures/block/stone.png"), malloc'd, caller frees.
// NULL when absent. (The C client's SpriteGetter/ModelBaker read through the
// pack exactly like the Java resource pipeline.)
unsigned char *LIBMATTI_MC_EmbeddedPackResources_ReadFile(LIBMATTI_MC_EmbeddedPackResources *pack,
                                                          const char *name, size_t *outLength);

// Java: the vanilla pack's known namespace count (the debug/log helper).
size_t LIBMATTI_MC_EmbeddedPackResources_EntryCount(const LIBMATTI_MC_EmbeddedPackResources *pack);

#endif //MATTICRAFT_MC_PACKS_EMBEDDED_EMBEDDEDPACKRESOURCES_H

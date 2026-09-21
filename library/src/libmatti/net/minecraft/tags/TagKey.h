// Port of net.minecraft.tags.TagKey.

#ifndef MATTICRAFT_MC_TAGS_TAGKEY_H
#define MATTICRAFT_MC_TAGS_TAGKEY_H

#include "libmatti/net/minecraft/resources/Identifier.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public record TagKey<T>(ResourceKey<? extends Registry<T>> registry, Identifier location)
// The record interns its instances (Java: Codec maps through an intern map); the
// port keeps the two identifiers and compares by value.
typedef struct LIBMATTI_MC_TagKey
{
    char *registry;   // Java: registry().location() - the registry the tag belongs to
    char *location;   // Java: location() - the tag itself, e.g. minecraft:planks
} LIBMATTI_MC_TagKey;

// Java: public static <T> TagKey<T> create(ResourceKey<? extends Registry<T>> registry, Identifier location)
LIBMATTI_MC_TagKey *LIBMATTI_MC_TagKey_Create(const char *registry, const char *location);
// Java: public static <T> TagKey<T> create(Identifier location) - over the default (item) registry
LIBMATTI_MC_TagKey *LIBMATTI_MC_TagKey_CreateItem(const char *location);
// Java: public static <T> TagKey<T> createBlockTag(Identifier location) - over the block registry
LIBMATTI_MC_TagKey *LIBMATTI_MC_TagKey_CreateBlock(const char *location);

// Value semantics (Java: record equals/hashCode)
int LIBMATTI_MC_TagKey_Equals(const LIBMATTI_MC_TagKey *a, const LIBMATTI_MC_TagKey *b);
void LIBMATTI_MC_TagKey_Free(LIBMATTI_MC_TagKey *tag);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_TAGS_TAGKEY_H

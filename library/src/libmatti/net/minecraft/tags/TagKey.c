// Port of net.minecraft.tags.TagKey.

#include "libmatti/net/minecraft/tags/TagKey.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final Map<TagKey<?>, TagKey<?>> INTERN_TABLE - the codec
// interns every parsed key. The port interns by location+registry string so the
// same tag shares one instance like in Java.
typedef struct TagKeyInterner
{
    LIBMATTI_MC_TagKey key;
    struct TagKeyInterner *next;
} TagKeyInterner;

static TagKeyInterner *intern_table;

static LIBMATTI_MC_TagKey *intern(const char *registry, const char *location)
{
    for (TagKeyInterner *entry = intern_table; entry != NULL; entry = entry->next)
    {
        if (strcmp(entry->key.registry, registry) == 0 && strcmp(entry->key.location, location) == 0)
            return &entry->key;
    }
    TagKeyInterner *entry = malloc(sizeof(TagKeyInterner));
    entry->key.registry = strdup(registry);
    entry->key.location = strdup(location);
    entry->next = intern_table;
    intern_table = entry;
    return &entry->key;
}

LIBMATTI_MC_TagKey *LIBMATTI_MC_TagKey_Create(const char *registry, const char *location)
{
    return intern(registry, location);
}

LIBMATTI_MC_TagKey *LIBMATTI_MC_TagKey_CreateItem(const char *location)
{
    return intern("minecraft:item", location);
}

LIBMATTI_MC_TagKey *LIBMATTI_MC_TagKey_CreateBlock(const char *location)
{
    return intern("minecraft:block", location);
}

int LIBMATTI_MC_TagKey_Equals(const LIBMATTI_MC_TagKey *a, const LIBMATTI_MC_TagKey *b)
{
    return strcmp(a->registry, b->registry) == 0 && strcmp(a->location, b->location) == 0;
}

void LIBMATTI_MC_TagKey_Free(LIBMATTI_MC_TagKey *tag)
{
    // Interned keys stay alive (Java's intern table does too); only NULL frees are no-ops.
    (void) tag;
}

// Port of net.minecraft.core.IdMapper - the id <-> value bijection
// (BlockColors keys its color providers by the block registry id through it).

#ifndef MATTICRAFT_CORE_IDMAPPER_H
#define MATTICRAFT_CORE_IDMAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: private final List<T> values / Int2ObjectMap<T> ids (T = the empty type).
// The port keeps both directions in parallel arrays (the map stays small).
typedef struct LIBMATTI_JL_IdMapper
{
    void **values; // insertion order
    int *ids;      // parallel: ids[i] is the id of values[i]
    int size;
    int capacity;
} LIBMATTI_JL_IdMapper;

// Creates an empty mapper. The mapper stores values by pointer and does not
// take ownership of them.
LIBMATTI_JL_IdMapper *LIBMATTI_JL_IdMapper_New(void);

// Frees the mapper's lookup storage, but not its mapped values. NULL is allowed.
void LIBMATTI_JL_IdMapper_Free(LIBMATTI_JL_IdMapper *mapper);

// Appends a value/id pair. Duplicate values and ids are retained; lookups
// return the first matching pair.
void LIBMATTI_JL_IdMapper_AddMapping(LIBMATTI_JL_IdMapper *mapper, void *value, int id);

// Returns the first value mapped to id, or NULL when id is unknown.
void *LIBMATTI_JL_IdMapper_ById(const LIBMATTI_JL_IdMapper *mapper, int id);

// Returns the id of the first pointer-identical value, or -1 when unknown.
int LIBMATTI_JL_IdMapper_GetId(const LIBMATTI_JL_IdMapper *mapper, const void *value);

// Returns the number of appended mappings, including duplicates.
int LIBMATTI_JL_IdMapper_Size(const LIBMATTI_JL_IdMapper *mapper);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_CORE_IDMAPPER_H

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

// Java: public IdMapper() - and IdMapper(int capacity) alike
LIBMATTI_JL_IdMapper *LIBMATTI_JL_IdMapper_New(void);
void LIBMATTI_JL_IdMapper_Free(LIBMATTI_JL_IdMapper *mapper);

// Java: public void addMapping(T value, int id)
void LIBMATTI_JL_IdMapper_AddMapping(LIBMATTI_JL_IdMapper *mapper, void *value, int id);

// Java: public T byId(int id) - NULL when unknown
void *LIBMATTI_JL_IdMapper_ById(const LIBMATTI_JL_IdMapper *mapper, int id);

// Java: public int getId(T value) - -1 when unknown
int LIBMATTI_JL_IdMapper_GetId(const LIBMATTI_JL_IdMapper *mapper, const void *value);

// Java: public int size()
int LIBMATTI_JL_IdMapper_Size(const LIBMATTI_JL_IdMapper *mapper);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_CORE_IDMAPPER_H

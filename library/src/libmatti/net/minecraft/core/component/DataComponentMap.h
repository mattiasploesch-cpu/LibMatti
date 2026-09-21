// Port of net.minecraft.core.component.DataComponentMap (plus DataComponentGetter).
// Java keys the map by DataComponentType identity (Reference2ObjectArrayMap); the C port
// keeps a parallel pointer/value array in insertion order - get() is a linear scan, the
// component counts per item are small. Values stay unowned void* (the map never frees).

#ifndef MATTICRAFT_MC_CORE_COMPONENT_DATACOMPONENTMAP_H
#define MATTICRAFT_MC_CORE_COMPONENT_DATACOMPONENTMAP_H

#include "libmatti/net/minecraft/core/component/DataComponentType.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public interface DataComponentMap extends DataComponentGetter
typedef struct LIBMATTI_MC_DataComponentMap
{
    LIBMATTI_MC_DataComponentType **types;
    void **values;
    size_t size;
    size_t capacity;
} LIBMATTI_MC_DataComponentMap;

// Java: DataComponentMap.EMPTY
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Empty(void);
// Java: DataComponentMap.builder().build()
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_New(void);

// Java: default <T> @Nullable T get(DataComponentType<? extends T>) - NULL when absent
void *LIBMATTI_MC_DataComponentMap_Get(const LIBMATTI_MC_DataComponentMap *map,
                                       const LIBMATTI_MC_DataComponentType *type);
// Java: default boolean has(DataComponentType<?>)
bool LIBMATTI_MC_DataComponentMap_Has(const LIBMATTI_MC_DataComponentMap *map,
                                      const LIBMATTI_MC_DataComponentType *type);
// Java: Builder.set - inserts or replaces; returns the map for chaining
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Set(LIBMATTI_MC_DataComponentMap *map,
                                                               const LIBMATTI_MC_DataComponentType *type,
                                                               void *value);
// Java: default int size() / isEmpty() / Set<DataComponentType<?>> keySet()
size_t LIBMATTI_MC_DataComponentMap_Size(const LIBMATTI_MC_DataComponentMap *map);
bool LIBMATTI_MC_DataComponentMap_IsEmpty(const LIBMATTI_MC_DataComponentMap *map);
// Java: Set<DataComponentType<?>> keySet() - the types array (caller does not free)
LIBMATTI_MC_DataComponentType **LIBMATTI_MC_DataComponentMap_KeySet(const LIBMATTI_MC_DataComponentMap *map,
                                                                    size_t *count);
// Java: static DataComponentMap composite(a, b) - b wins over a
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Composite(const LIBMATTI_MC_DataComponentMap *a,
                                                                     const LIBMATTI_MC_DataComponentMap *b);
// C-port deep copy (Java's PatchedDataComponentMap.copy uses this)
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Copy(const LIBMATTI_MC_DataComponentMap *map);
void LIBMATTI_MC_DataComponentMap_Free(LIBMATTI_MC_DataComponentMap *map);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CORE_COMPONENT_DATACOMPONENTMAP_H

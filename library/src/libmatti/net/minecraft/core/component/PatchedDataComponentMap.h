// Port of net.minecraft.core.component.PatchedDataComponentMap and DataComponentPatch.
// Java's patched map layers an Optional-valued patch over the item's prototype map:
// a present Optional overrides, an empty Optional removes. The C port keeps the same
// semantics with a parallel (type, present, value) patch array over a prototype map.
// DataComponentPatch is the same (type, present, value) list without a prototype.

#ifndef MATTICRAFT_MC_CORE_COMPONENT_PATCHEDDATACOMPONENTMAP_H
#define MATTICRAFT_MC_CORE_COMPONENT_PATCHEDDATACOMPONENTMAP_H

#include "libmatti/net/minecraft/core/component/DataComponentMap.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LIBMATTI_MC_DataComponentPatch LIBMATTI_MC_DataComponentPatch;

// Java: public final class PatchedDataComponentMap implements DataComponentMap
typedef struct LIBMATTI_MC_PatchedDataComponentMap
{
    // Java: private final DataComponentMap prototype
    const LIBMATTI_MC_DataComponentMap *prototype;
    // Java: Reference2ObjectMap<DataComponentType<?>, Optional<?>> patch
    LIBMATTI_MC_DataComponentType **patchTypes;
    bool *patchPresent;
    void **patchValues;
    size_t patchSize;
    size_t patchCapacity;
    // Java: boolean copyOnWrite
    bool copyOnWrite;
} LIBMATTI_MC_PatchedDataComponentMap;

// Java: public PatchedDataComponentMap(DataComponentMap prototype)
LIBMATTI_MC_PatchedDataComponentMap *LIBMATTI_MC_PatchedDataComponentMap_New(
        const LIBMATTI_MC_DataComponentMap *prototype);
// Java: public static PatchedDataComponentMap fromPatch(DataComponentMap, DataComponentPatch)
LIBMATTI_MC_PatchedDataComponentMap *LIBMATTI_MC_PatchedDataComponentMap_FromPatch(
        const LIBMATTI_MC_DataComponentMap *prototype, const LIBMATTI_MC_DataComponentPatch *patch);

// Java: public <T> @Nullable T get(DataComponentType<? extends T>)
void *LIBMATTI_MC_PatchedDataComponentMap_Get(const LIBMATTI_MC_PatchedDataComponentMap *map,
                                              const LIBMATTI_MC_DataComponentType *type);
// Java: public boolean hasNonDefault(DataComponentType<?>)
bool LIBMATTI_MC_PatchedDataComponentMap_HasNonDefault(const LIBMATTI_MC_PatchedDataComponentMap *map,
                                                       const LIBMATTI_MC_DataComponentType *type);
// Java: public <T> @Nullable T set(DataComponentType<T>, @Nullable T)
void *LIBMATTI_MC_PatchedDataComponentMap_Set(LIBMATTI_MC_PatchedDataComponentMap *map,
                                              const LIBMATTI_MC_DataComponentType *type, void *value);
// Java: public <T> @Nullable T remove(DataComponentType<? extends T>)
void *LIBMATTI_MC_PatchedDataComponentMap_Remove(LIBMATTI_MC_PatchedDataComponentMap *map,
                                                 const LIBMATTI_MC_DataComponentType *type);
// Java: public void clearPatch()
void LIBMATTI_MC_PatchedDataComponentMap_ClearPatch(LIBMATTI_MC_PatchedDataComponentMap *map);
// Java: public void applyPatch(DataComponentPatch) / restorePatch(DataComponentPatch)
void LIBMATTI_MC_PatchedDataComponentMap_ApplyPatch(LIBMATTI_MC_PatchedDataComponentMap *map,
                                                    const LIBMATTI_MC_DataComponentPatch *patch);
void LIBMATTI_MC_PatchedDataComponentMap_RestorePatch(LIBMATTI_MC_PatchedDataComponentMap *map,
                                                      const LIBMATTI_MC_DataComponentPatch *patch);
LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_PatchedDataComponentMap_AsPatch(
        const LIBMATTI_MC_PatchedDataComponentMap *map);
// Java: public PatchedDataComponentMap copy()
LIBMATTI_MC_PatchedDataComponentMap *LIBMATTI_MC_PatchedDataComponentMap_Copy(
        const LIBMATTI_MC_PatchedDataComponentMap *map);
// C-port ownership drop (the map itself, never the values)
void LIBMATTI_MC_PatchedDataComponentMap_Free(LIBMATTI_MC_PatchedDataComponentMap *map);
// Java: keySet() - the union of prototype and patch keys (set semantics over an array)
LIBMATTI_MC_DataComponentType **LIBMATTI_MC_PatchedDataComponentMap_KeySet(
        const LIBMATTI_MC_PatchedDataComponentMap *map, size_t *count);
// Java: Objects.equals(map, other) - patch and prototype must resolve to the same values
bool LIBMATTI_MC_PatchedDataComponentMap_Equals(const LIBMATTI_MC_PatchedDataComponentMap *a,
                                                const LIBMATTI_MC_PatchedDataComponentMap *b);

// Java: public final class DataComponentPatch
typedef struct LIBMATTI_MC_DataComponentPatch
{
    LIBMATTI_MC_DataComponentType **types;
    bool *present;
    void **values;
    size_t size;
    size_t capacity;
} LIBMATTI_MC_DataComponentPatch;

// Java: public static final DataComponentPatch EMPTY
LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_DataComponentPatch_Empty(void);
LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_DataComponentPatch_New(void);
// Java: the map builder side - set(type, value) / remove(type)
void LIBMATTI_MC_DataComponentPatch_Set(LIBMATTI_MC_DataComponentPatch *patch,
                                        const LIBMATTI_MC_DataComponentType *type, void *value);
void LIBMATTI_MC_DataComponentPatch_Remove(LIBMATTI_MC_DataComponentPatch *patch,
                                           const LIBMATTI_MC_DataComponentType *type);
// Java: public boolean isEmpty()
bool LIBMATTI_MC_DataComponentPatch_IsEmpty(const LIBMATTI_MC_DataComponentPatch *patch);
void LIBMATTI_MC_DataComponentPatch_Free(LIBMATTI_MC_DataComponentPatch *patch);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CORE_COMPONENT_PATCHEDDATACOMPONENTMAP_H

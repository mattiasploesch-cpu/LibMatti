// Port of net.minecraft.core.MappedRegistry (also serving as WritableRegistry, the Holder
// machinery folds into the registry: a Reference holder is the registry's own entry record).
// Java keys values by identity (IdentityHashMap/Reference2IntMap); the C port does the same
// with pointer keys. Tags (frozenTags/allTags) are the tag-loading part and stay out until
// the game port needs them.

#ifndef MATTICRAFT_NET_MINECRAFT_CORE_MAPPEDREGISTRY_H
#define MATTICRAFT_NET_MINECRAFT_CORE_MAPPEDREGISTRY_H

#include "libmatti/com/mojang/serialization/Lifecycle.h"
#include "libmatti/net/minecraft/core/RegistrationInfo.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"

#include <stddef.h>

// Java: Holder.Reference<T> - the registry's entry record; the holder binds key and value
typedef struct LIBMATTI_MC_HolderReference
{
    // Java: the owning registry (Holder.Reference.createStandAlone(this, key))
    struct LIBMATTI_MC_MappedRegistry *registry;
    // Java: bindKey(ResourceKey) - NULL until the value is registered
    LIBMATTI_MC_ResourceKey *key;
    // Java: bindValue(T) on freeze - NULL until bound
    void *value;
    // Java: isBound() - key != null && value != null
    int bound;
    // Java: bindTags(List<TagKey>) - the tag keys bound to this holder
    LIBMATTI_MC_ResourceKey **tags;
    size_t tagCount;
} LIBMATTI_MC_HolderReference;

// Java: public class MappedRegistry<T> implements WritableRegistry<T>
typedef struct LIBMATTI_MC_MappedRegistry LIBMATTI_MC_MappedRegistry;

struct LIBMATTI_MC_MappedRegistry
{
    // Java: private final ResourceKey<? extends Registry<T>> key;
    LIBMATTI_MC_ResourceKey *key;
    // Java: private final ObjectList<Holder.Reference<T>> byId;
    LIBMATTI_MC_HolderReference **byId;
    size_t byIdCount;
    size_t byIdCapacity;
    // Java: private final Reference2IntMap<T> toId (default -1)
    void **toIdKeys;
    int *toIdValues;
    size_t toIdCount;
    size_t toIdCapacity;
    // Java: private final Map<Identifier, Holder.Reference<T>> byLocation;
    struct
    {
        LIBMATTI_MC_Identifier **keys;
        LIBMATTI_MC_HolderReference **values;
        size_t count;
        size_t capacity;
    } byLocation;
    // Java: private final Map<ResourceKey<T>, Holder.Reference<T>> byKey;
    struct
    {
        LIBMATTI_MC_ResourceKey **keys;
        LIBMATTI_MC_HolderReference **values;
        size_t count;
        size_t capacity;
    } byKey;
    // Java: private final Map<T, Holder.Reference<T>> byValue (identity);
    struct
    {
        void **keys;
        LIBMATTI_MC_HolderReference **values;
        size_t count;
        size_t capacity;
    } byValue;
    // Java: private final Map<ResourceKey<T>, RegistrationInfo> registrationInfos;
    struct
    {
        LIBMATTI_MC_ResourceKey **keys;
        LIBMATTI_MC_RegistrationInfo *values;
        size_t count;
        size_t capacity;
    } registrationInfos;
    // Java: private Lifecycle registryLifecycle;
    LIBMATTI_MJS_Lifecycle registryLifecycle;
    // Java: private boolean frozen;
    int frozen;
    // Java: intrusive holders (unregisteredIntrusiveHolders != null when intrusive)
    int intrusiveHolders;
    // Java: the default key of DefaultedMappedRegistry - NULL for plain registries
    char *defaultKey;
};

// Java: public MappedRegistry(ResourceKey<? extends Registry<T>> key, Lifecycle lifecycle,
//                             boolean intrusive) - the C port always passes the ResourceKey
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_MappedRegistry_New(LIBMATTI_MC_ResourceKey *key,
                                                            LIBMATTI_MJS_Lifecycle lifecycle,
                                                            int intrusiveHolders);
void LIBMATTI_MC_MappedRegistry_Free(LIBMATTI_MC_MappedRegistry *registry);

// Java: public ResourceKey<? extends Registry<T>> key()
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_MappedRegistry_Key(const LIBMATTI_MC_MappedRegistry *registry);

// Java: private void validateWrite(ResourceKey) - aborts when frozen (Java throws)
void LIBMATTI_MC_MappedRegistry_ValidateWrite(const LIBMATTI_MC_MappedRegistry *registry);

// Java: public Holder.Reference<T> register(ResourceKey<T> key, T value, RegistrationInfo info)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_Register(LIBMATTI_MC_MappedRegistry *registry,
                                                                 LIBMATTI_MC_ResourceKey *resourceKey,
                                                                 void *value,
                                                                 LIBMATTI_MC_RegistrationInfo info);

// Java: @Nullable public Identifier getKey(T value) - NULL when unregistered
LIBMATTI_MC_Identifier *LIBMATTI_MC_MappedRegistry_GetKey(const LIBMATTI_MC_MappedRegistry *registry, const void *value);
// Java: public Optional<ResourceKey<T>> getResourceKey(T value)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_MappedRegistry_GetResourceKey(const LIBMATTI_MC_MappedRegistry *registry,
                                                                   const void *value);
// Java: public int getId(@Nullable T value) - -1 when missing
int LIBMATTI_MC_MappedRegistry_GetId(const LIBMATTI_MC_MappedRegistry *registry, const void *value);
// Java: @Nullable public T getValue(@Nullable ResourceKey<T> key)
void *LIBMATTI_MC_MappedRegistry_GetValueByKey(const LIBMATTI_MC_MappedRegistry *registry,
                                               const LIBMATTI_MC_ResourceKey *resourceKey);
// Java: @Nullable public T getValue(@Nullable Identifier location)
void *LIBMATTI_MC_MappedRegistry_GetValue(const LIBMATTI_MC_MappedRegistry *registry,
                                          const LIBMATTI_MC_Identifier *location);
// Java: @Nullable public T byId(int id)
void *LIBMATTI_MC_MappedRegistry_ById(const LIBMATTI_MC_MappedRegistry *registry, int id);
// Java: public Optional<Holder.Reference<T>> get(int id)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetHolderById(const LIBMATTI_MC_MappedRegistry *registry, int id);
// Java: public Optional<Holder.Reference<T>> get(Identifier location)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetHolderByLocation(const LIBMATTI_MC_MappedRegistry *registry,
                                                                            const LIBMATTI_MC_Identifier *location);
// Java: public Optional<Holder.Reference<T>> get(ResourceKey<T> key)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetHolderByKey(const LIBMATTI_MC_MappedRegistry *registry,
                                                                       const LIBMATTI_MC_ResourceKey *resourceKey);
// Java: public Optional<Holder.Reference<T>> getAny() - the first entry
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetAny(const LIBMATTI_MC_MappedRegistry *registry);

// Java: public int size()
int LIBMATTI_MC_MappedRegistry_Size(const LIBMATTI_MC_MappedRegistry *registry);
// Java: public boolean isEmpty()
int LIBMATTI_MC_MappedRegistry_IsEmpty(const LIBMATTI_MC_MappedRegistry *registry);

// Java: public Optional<RegistrationInfo> registrationInfo(ResourceKey<T> key) - NULL when absent
const LIBMATTI_MC_RegistrationInfo *LIBMATTI_MC_MappedRegistry_RegistrationInfo(const LIBMATTI_MC_MappedRegistry *registry,
                                                                                const LIBMATTI_MC_ResourceKey *resourceKey);
// Java: public Lifecycle registryLifecycle()
LIBMATTI_MJS_Lifecycle LIBMATTI_MC_MappedRegistry_RegistryLifecycle(const LIBMATTI_MC_MappedRegistry *registry);

// Java: public Iterator<T> iterator() - the C port exposes the byId array of holders
LIBMATTI_MC_HolderReference **LIBMATTI_MC_MappedRegistry_Iterators(const LIBMATTI_MC_MappedRegistry *registry,
                                                                   size_t *count);
// Java: public Set<Identifier> keySet()
LIBMATTI_MC_Identifier **LIBMATTI_MC_MappedRegistry_KeySet(const LIBMATTI_MC_MappedRegistry *registry, size_t *count);
// Java: public Set<ResourceKey<T>> registryKeySet()
LIBMATTI_MC_ResourceKey **LIBMATTI_MC_MappedRegistry_RegistryKeySet(const LIBMATTI_MC_MappedRegistry *registry,
                                                                    size_t *count);

// Java: public boolean containsKey(Identifier) / containsKey(ResourceKey)
int LIBMATTI_MC_MappedRegistry_ContainsLocation(const LIBMATTI_MC_MappedRegistry *registry,
                                                const LIBMATTI_MC_Identifier *location);
int LIBMATTI_MC_MappedRegistry_ContainsKey(const LIBMATTI_MC_MappedRegistry *registry,
                                           const LIBMATTI_MC_ResourceKey *resourceKey);

// Java: public Registry<T> freeze() - binds every value, aborts on unbound entries
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_MappedRegistry_Freeze(LIBMATTI_MC_MappedRegistry *registry);

// Java: void unfreeze(boolean usingWrittenKeys) - GameData.unfreezeData lifts the guard so the
// RegisterEvents can write
void LIBMATTI_MC_MappedRegistry_Unfreeze(LIBMATTI_MC_MappedRegistry *registry);

// DefaultedRegistry part (DefaultedMappedRegistry wraps a MappedRegistry with a default key):
// Java: public Identifier getDefaultKey()
const char *LIBMATTI_MC_MappedRegistry_GetDefaultKey(const LIBMATTI_MC_MappedRegistry *registry);
// Java: @NonNull public T getValue(@Nullable Identifier) - falls back to the default value
void *LIBMATTI_MC_MappedRegistry_GetValueOrDefault(const LIBMATTI_MC_MappedRegistry *registry,
                                                   const LIBMATTI_MC_Identifier *location);
// Java: @NonNull public T byId(int) - falls back to the default value
void *LIBMATTI_MC_MappedRegistry_ByIdOrDefault(const LIBMATTI_MC_MappedRegistry *registry, int id);

// The port's DefaultedMappedRegistry constructor: DefaultedRegistry<T> with getDefaultKey()
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_DefaultedMappedRegistry_New(const char *defaultKey,
                                                                    LIBMATTI_MC_ResourceKey *key,
                                                                    LIBMATTI_MJS_Lifecycle lifecycle,
                                                                    int intrusiveHolders);

#endif //MATTICRAFT_NET_MINECRAFT_CORE_MAPPEDREGISTRY_H

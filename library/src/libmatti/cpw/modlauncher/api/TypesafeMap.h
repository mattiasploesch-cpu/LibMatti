//
// Port of cpw.mods.modlauncher.api.TypesafeMap.
// Java uses java.lang.Class as the type identity; here the class name string
// takes its place (Key.getOrCreate compares it).
//

#ifndef MATTICRAFT_MODLAUNCHER_TYPESAFEMAP_H
#define MATTICRAFT_MODLAUNCHER_TYPESAFEMAP_H

#include <stddef.h>

typedef struct LIBMATTI_MLA_TypesafeMap LIBMATTI_MLA_TypesafeMap;
typedef struct LIBMATTI_MLA_Key LIBMATTI_MLA_Key;

// Java: static <V> Key<V> getOrCreate(TypesafeMap owner, String name, Class<? super V> clazz)
LIBMATTI_MLA_Key *LIBMATTI_MLA_Key_GetOrCreate(LIBMATTI_MLA_TypesafeMap *owner, const char *name, const char *clazz);
// Java: String name()
const char *LIBMATTI_MLA_Key_Name(const LIBMATTI_MLA_Key *key);

// Java: TypesafeMap()
LIBMATTI_MLA_TypesafeMap *LIBMATTI_MLA_TypesafeMap_New(void);
// Java: TypesafeMap(Class<?> owner) - builds the keys registered for the owner
LIBMATTI_MLA_TypesafeMap *LIBMATTI_MLA_TypesafeMap_NewOwned(const char *owner);
// Java: Optional<V> get(Key<V>) - NULL means Optional.empty()
void *LIBMATTI_MLA_TypesafeMap_Get(const LIBMATTI_MLA_TypesafeMap *map, LIBMATTI_MLA_Key *key);
// Java: V computeIfAbsent(Key<V> key, Function<? super Key<V>, ? extends V> valueFunction)
void *LIBMATTI_MLA_TypesafeMap_ComputeIfAbsent(LIBMATTI_MLA_TypesafeMap *map, LIBMATTI_MLA_Key *key,
                                               void *(*valueFunction)(LIBMATTI_MLA_Key *key, void *userdata),
                                               void *userdata);

// Java: static final class KeyBuilder<T> implements Supplier<Key<T>>
typedef struct LIBMATTI_MLA_KeyBuilder LIBMATTI_MLA_KeyBuilder;

// Java: KeyBuilder(String name, Class<? super T> clazz, Class<?> owner)
LIBMATTI_MLA_KeyBuilder *LIBMATTI_MLA_KeyBuilder_New(const char *name, const char *clazz, const char *owner);
// Java: Key<T> get()
LIBMATTI_MLA_Key *LIBMATTI_MLA_KeyBuilder_Get(LIBMATTI_MLA_KeyBuilder *builder);

#endif //MATTICRAFT_MODLAUNCHER_TYPESAFEMAP_H

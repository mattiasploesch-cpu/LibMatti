// Port of net.minecraft.core.component.DataComponentType.
// Java is an interface over (codec, streamCodec, ignoreSwapAnimation); the C port
// keeps the registry-data part: the registered name, the transient flag and the
// swap-animation flag. The codecs stay with the game port (values are void*).

#ifndef MATTICRAFT_MC_CORE_COMPONENT_DATACOMPONENTTYPE_H
#define MATTICRAFT_MC_CORE_COMPONENT_DATACOMPONENTTYPE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public interface DataComponentType<T> - built through DataComponentType.builder()
typedef struct LIBMATTI_MC_DataComponentType
{
    // Java: the registered name (Registry.register(BuiltInRegistries.DATA_COMPONENT_TYPE, ...))
    char *name;
    // Java: @Nullable Codec<T> codec() - NULL means transient
    void *codec;
    // Java: boolean ignoreSwapAnimation()
    bool ignoreSwapAnimation;
} LIBMATTI_MC_DataComponentType;

// Java: DataComponentType.builder().persistent(...)...build() - the port takes the
// registry name (registered through the DataComponents bootstrap) and the flags
LIBMATTI_MC_DataComponentType *LIBMATTI_MC_DataComponentType_Create(const char *name, void *codec);
LIBMATTI_MC_DataComponentType *LIBMATTI_MC_DataComponentType_CreateTransient(const char *name);
// Java: default boolean isTransient() - codec == null
bool LIBMATTI_MC_DataComponentType_IsTransient(const LIBMATTI_MC_DataComponentType *type);
// Java: default Codec<T> codecOrThrow() - aborts on a transient type (Java throws)
void *LIBMATTI_MC_DataComponentType_CodecOrThrow(const LIBMATTI_MC_DataComponentType *type);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CORE_COMPONENT_DATACOMPONENTTYPE_H

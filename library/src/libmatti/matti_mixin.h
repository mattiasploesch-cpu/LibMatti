























#ifndef MATTICRAFT_MATTI_MIXIN_H
#define MATTICRAFT_MATTI_MIXIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif



#define MATTI_MIXIN_SECTION ".matti_mixins"

// Java: @Inject(at = @At("HEAD")) etc.
typedef enum
{
    MATTI_MIXIN_AT_HEAD = 0,
    MATTI_MIXIN_AT_RETURN,
    MATTI_MIXIN_AT_TAIL
} MattiMixinAt;

// Java: MixinPriority, encoded as DEFAULT=1000 so mods can pass e.g. 1000 +/- an offset
#define MATTI_MIXIN_PRIORITY_DEFAULT 1000

// Java: InjectionPoint / CallbackInfo#isCancelled
typedef enum
{
    MATTI_MIXIN_PASS = 0,
    MATTI_MIXIN_CANCEL
} MattiMixinResult;

// Java: the CallbackInfo the handler receives
typedef struct MattiMixinCallbackInfo
{
    const char *target;

    void *owner;

    void **args;

    size_t argCount;
    MattiMixinResult result;
} MattiMixinCallbackInfo;

// Java: the injector signature - the handler decides through info->result whether to cancel
typedef void (*MattiMixinHandler)(MattiMixinCallbackInfo *info, void *userdata);



typedef struct MattiMixinEntry
{
    const char *target;

    MattiMixinHandler handler;
    void *userdata;
    int at;

    int priority;

    const char *modId;

} MattiMixinEntry;

// Java: a mixin class declares an injection. The C mixin declares its handler:
//
//     static void my_handler(MattiMixinCallbackInfo *info, void *userdata) { ... }
//     MATTI_MIXIN("examplemod", "net.minecraft.client.Minecraft::runTick",
//                 my_handler, NULL, MATTI_MIXIN_AT_HEAD, MATTI_MIXIN_PRIORITY_DEFAULT)
// aligned(8) keeps the section a packed array: GCC otherwise aligns every static to its own
// (larger) boundary, which would leave gaps the collector cannot skip.
#define MATTI_MIXIN(modId, targetName, handlerFunction, handlerUserdata, atPoint, priorityValue)             \
    __attribute__((used, section(MATTI_MIXIN_SECTION), aligned(8)))                                          \
    static const MattiMixinEntry MattiMixinEntry_##handlerFunction = {                                       \
        targetName, handlerFunction, handlerUserdata, (atPoint), (priorityValue), modId};









#define MATTI_MIXIN_TARGET(targetName, function)                                                             \
    __attribute__((used, section(MATTI_MIXIN_SECTION), aligned(8)))                                          \
    static const MattiMixinEntry MattiMixinTargetEntry_##function = {                                        \
        targetName, (MattiMixinHandler) (function), NULL, MATTI_MIXIN_AT_HEAD, 0, NULL};

#ifdef __cplusplus
}
#endif

#endif

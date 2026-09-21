// Matticraft mixins in C (backend M4: the hook table).
//
// Java weaves mixins into Java bytecode while the JVM defines the class. A Matticraft "class" is
// native code in a shared object, so there is no bytecode to weave - the weave point is the call
// itself. The game (and every mod) declares a mixable function with MATTI_MIXIN_TARGET and calls it
// through the resolved hook chain; a mod hooks into it with MATTI_MIXIN.
//
// Both macros put one descriptor into the ELF section ".matti_mixins" of the shared object they are
// compiled into (the same mechanism as the ".matti_manifest" section). At mixin setup the loader
// collects the descriptors of the game binary and of every loaded mod file and chains them per
// target: the priority ("phase") orders the handlers, and the chain head receives the invocation.
//
// A handler runs in the Java mixin points HEAD (before the target), TAIL (after the target) or
// RETURN (replacing the return value decision); CANCELLED declares whether the handler wants to
// cancel the remaining chain. The mixable call reports what happened:
//
//     MATTI_MIXIN_PASS     every handler ran, no one cancelled
//     MATTI_MIXIN_CANCEL   a handler cancelled the invocation
//
// The resolved chain is read with LIBMATTI_MIXIN_Lookup; the target owner dispatches over it with
// LIBMATTI_MIXIN_Invoke. A target no mod hooked still works - its chain is just empty and the call
// goes straight through.

#ifndef MATTICRAFT_MATTI_MIXIN_H
#define MATTICRAFT_MATTI_MIXIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// The ELF section the descriptors land in
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
    const char *target;   // the mixed-in target function's name
    void *owner;          // the self pointer the target was invoked with
    void **args;          // the argument vector of the invocation
    size_t argCount;
    MattiMixinResult result;
} MattiMixinCallbackInfo;

// Java: the injector signature - the handler decides through info->result whether to cancel
typedef void (*MattiMixinHandler)(MattiMixinCallbackInfo *info, void *userdata);

// One descriptor, exactly what the two macros put into .matti_mixins
typedef struct MattiMixinEntry
{
    const char *target;   // the mixable function's name, as given to MATTI_MIXIN_TARGET
    MattiMixinHandler handler;
    void *userdata;
    int at;               // MattiMixinAt
    int priority;         // lower runs earlier
    const char *modId;    // the mixin's owner, for the audit log
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

// The counterpart in the shared object that owns the mixable function:
//
//     static void game_tick(void) { ... }
//     MATTI_MIXIN_TARGET("net.minecraft.client.Minecraft::runTick", game_tick)
//
// The symbol is emitted exactly once (the definition), and the descriptor carries its address so the
// loader can find the owner of a target without a symbol lookup.
#define MATTI_MIXIN_TARGET(targetName, function)                                                             \
    __attribute__((used, section(MATTI_MIXIN_SECTION), aligned(8)))                                          \
    static const MattiMixinEntry MattiMixinTargetEntry_##function = {                                        \
        targetName, (MattiMixinHandler) (function), NULL, MATTI_MIXIN_AT_HEAD, 0, NULL};

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MATTI_MIXIN_H

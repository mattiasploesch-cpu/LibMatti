// Port-only: the M4 + M2 mixin backends. See matti_mixin.h for the descriptor macros.
//
// M4 (hook table): every mixable call goes through the table. The game owns the table for its own
// targets; a mod reads the table through the same header set and dispatches its own targets the same
// way. Resolution maps a target name to the chain of mixin entries (handler, phase, priority).
//
// M2 (GOT patching): for symbols the game *imports* (libc, other shared objects), the loader rewrites
// the GOT slots of the loaded mod files so calls to that symbol land on a mixin handler. The original
// address is kept and re-invocable (Java: the "target" of an @Inject at HEAD).

#ifndef MATTICRAFT_MIXIN_HOOKTABLE_H
#define MATTICRAFT_MIXIN_HOOKTABLE_H

#include "libmatti/matti_mixin.h"

#include <stddef.h>

typedef struct LIBMATTI_MIXIN_HookTable LIBMATTI_MIXIN_HookTable;

// One resolved handler of a target's chain
typedef struct
{
    MattiMixinHandler handler;
    void *userdata;
    MattiMixinAt at;
    int priority;
    const char *modId;
} LIBMATTI_MIXIN_Hook;

// The chain of one target, in execution order
typedef struct
{
    const char *target;
    // M4: the address MATTI_MIXIN_TARGET emitted for the target (the "original")
    void *original;
    LIBMATTI_MIXIN_Hook *hooks;
    size_t hookCount;
} LIBMATTI_MIXIN_Target;

// Java: MixinEnvironment.gotoPhase - the setup the facade drives
LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_New(void);

// Java: MixinEnvironment holds the process-wide setup; the facade collects into this table and mods
// dispatch over it (LIBMATTI_MIXIN_Invoke(LIBMATTI_MIXIN_HookTable_Default(), ...)).
LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_Default(void);

// M4: collect the descriptors of one shared object (the game executable or a mod) into the table.
// 'path' is read like the manifest section is, so both the main binary and .so mod files work.
// Returns the number of descriptors taken from the object.
size_t LIBMATTI_MIXIN_HookTable_AddObject(LIBMATTI_MIXIN_HookTable *table, const char *path, const char *modId);

// M4: register one target's original address (what MATTI_MIXIN_TARGET emitted). The owner calls this
// once per target at startup; hook entries registered for the same name attach to it.
void LIBMATTI_MIXIN_HookTable_RegisterTarget(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                             void *original);

// M4: register one hook entry at run time (what AddObject does per descriptor)
void LIBMATTI_MIXIN_HookTable_AddHook(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                      MattiMixinHandler handler, void *userdata, MattiMixinAt at,
                                      int priority, const char *modId);

// M4: the resolved chain of a target, ordered by priority; NULL when nobody mixed into it
const LIBMATTI_MIXIN_Target *LIBMATTI_MIXIN_Lookup(const LIBMATTI_MIXIN_HookTable *table,
                                                   const char *target);

// M4: dispatch one invocation over the target's chain. 'result' is MATTI_MIXIN_CANCEL when a handler
// cancelled, otherwise MATTI_MIXIN_PASS; an empty chain is a pass. Returns 1 when the owner should
// still run the original body (HEAD that did not cancel), 0 otherwise.
int LIBMATTI_MIXIN_Invoke(const LIBMATTI_MIXIN_HookTable *table, const char *target, void *owner,
                          void **args, size_t argCount, MattiMixinResult *result);

// M2: rewrite the GOT of the loaded object at 'path' so every call to a symbol listed in
// 'symbolNames' (count 'symbolCount') lands on 'hook'. The previous address is remembered; the
// callback form lets the wrapper call the original. Returns the number of rewritten slots.
size_t LIBMATTI_MIXIN_GotPatch(const char *path, const char *const *symbolNames, size_t symbolCount,
                               MattiMixinHandler hook, void *userdata, const char *modId);

// M2: the original address a GOT slot held before the patch (for re-invoking the target)
void *LIBMATTI_MIXIN_GotOriginal(const char *path, const char *symbolName);

void LIBMATTI_MIXIN_HookTable_Free(LIBMATTI_MIXIN_HookTable *table);

#endif //MATTICRAFT_MIXIN_HOOKTABLE_H

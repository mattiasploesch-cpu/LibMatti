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



typedef struct
{
    MattiMixinHandler handler;
    void *userdata;
    MattiMixinAt at;
    int priority;
    const char *modId;
} LIBMATTI_MIXIN_Hook;



typedef struct
{
    const char *target;


    void *original;
    LIBMATTI_MIXIN_Hook *hooks;
    size_t hookCount;
} LIBMATTI_MIXIN_Target;

// Java: MixinEnvironment.gotoPhase - the setup the facade drives
LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_New(void);

// Java: MixinEnvironment holds the process-wide setup; the facade collects into this table and mods
// dispatch over it (LIBMATTI_MIXIN_Invoke(LIBMATTI_MIXIN_HookTable_Default(), ...)).
LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_Default(void);





size_t LIBMATTI_MIXIN_HookTable_AddObject(LIBMATTI_MIXIN_HookTable *table, const char *path, const char *modId);




void LIBMATTI_MIXIN_HookTable_RegisterTarget(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                             void *original);



void LIBMATTI_MIXIN_HookTable_AddHook(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                      MattiMixinHandler handler, void *userdata, MattiMixinAt at,
                                      int priority, const char *modId);



const LIBMATTI_MIXIN_Target *LIBMATTI_MIXIN_Lookup(const LIBMATTI_MIXIN_HookTable *table,
                                                   const char *target);





int LIBMATTI_MIXIN_Invoke(const LIBMATTI_MIXIN_HookTable *table, const char *target, void *owner,
                          void **args, size_t argCount, MattiMixinResult *result);





size_t LIBMATTI_MIXIN_GotPatch(const char *path, const char *const *symbolNames, size_t symbolCount,
                               MattiMixinHandler hook, void *userdata, const char *modId);



void *LIBMATTI_MIXIN_GotOriginal(const char *path, const char *symbolName);

void LIBMATTI_MIXIN_HookTable_Free(LIBMATTI_MIXIN_HookTable *table);

#endif

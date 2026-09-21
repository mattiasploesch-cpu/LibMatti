// Throwaway proof for the M4 + M2 mixin backends:
// - a NON-PIE executable declares the game target the example mod hooks into
// - a mod .so (the real examplemod, built by the mods project) hooks it via MATTI_MIXIN
// - the hook table is collected exactly like MixinFacade does (game binary + mod files)
// - a runtime hook proves HEAD cancellation (Java: ci.cancel() at @At("HEAD"))
// - M2: the executable's GOT slot for a libc symbol is rewritten to a handler
// Build (non-PIE on purpose - it exercises the l_addr == 0 path):
//   cc -g -no-pie -I library/src -o /tmp/mcscan/m4proof /tmp/mcscan/m4proof.c \
//        cmake-build-debug/library/libmatti.a -ldl
//   valgrind --error-exitcode=9 --leak-check=full /tmp/mcscan/m4proof mods/examplemod-1.0.so

#include "libmatti/matti/mixin/MixinHookTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tickBodyRan = 0;
static int runtimeHookRan = 0;

// The target the example mod mixes into (client/src/main.c declares the same name)
static void game_tick(void)
{
    tickBodyRan = 1;
    printf("target body ran\n");
}

MATTI_MIXIN_TARGET("matticraft::demo::tick", game_tick)

static void unused_handler(MattiMixinCallbackInfo *info, void *userdata)
{
    (void) info;
    (void) userdata;
}

// Fills the executable's own descriptor section alongside the target entry
MATTI_MIXIN("host", "unused.target", unused_handler, NULL, MATTI_MIXIN_AT_HEAD, 1)

// Runtime-registered hook, lower priority than DEFAULT: runs before the mod hook and cancels
static void cancelling_hook(MattiMixinCallbackInfo *info, void *userdata)
{
    (void) userdata;
    runtimeHookRan = 1;
    info->result = MATTI_MIXIN_CANCEL;
}

// The libc call the executable imports; M2 rewrites its GOT slot to this handler, which
// re-invokes the original (the documented M2 re-invocation path) and flags the rewrite.
static int m2Patched = 0;
static void puts_handler(MattiMixinCallbackInfo *info, void *userdata)
{
    (void) info;
    (void) userdata;
    m2Patched = 1;
    int (*original)(const char *) = LIBMATTI_MIXIN_GotOriginal("/proc/self/exe", "puts");
    if (original != NULL)
        original("M2: puts patched, original re-invoked");
}

static int failures = 0;

#define CHECK(condition, message)                                              \
    do                                                                         \
    {                                                                          \
        if (condition)                                                         \
            printf("OK  %s\n", message);                                       \
        else                                                                   \
        {                                                                      \
            printf("FAIL %s\n", message);                                      \
            failures++;                                                        \
        }                                                                      \
    } while (0)

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: m4proof <mod.so>\n");
        return 2;
    }

    LIBMATTI_MIXIN_HookTable *table = LIBMATTI_MIXIN_HookTable_New();

    // MixinFacade's collection order: the game binary first, then every mod file
    size_t gameTargets = LIBMATTI_MIXIN_HookTable_AddObject(table, NULL, NULL);
    size_t modHooks = LIBMATTI_MIXIN_HookTable_AddObject(table, argv[1], "classmod");
    printf("collected %zu game target(s), %zu mod hook(s)\n", gameTargets, modHooks);
    CHECK(gameTargets == 2, "game binary: target entry + host hook collected");
    CHECK(modHooks == 1, "mod .so: one hook descriptor collected");

    const LIBMATTI_MIXIN_Target *chain = LIBMATTI_MIXIN_Lookup(table, "matticraft::demo::tick");
    CHECK(chain != NULL && chain->hookCount == 1 && chain->original == (void *) game_tick &&
              chain->hooks[0].modId != NULL && strcmp(chain->hooks[0].modId, "classmod") == 0,
          "chain resolved: original bound, hook attributed to classmod");

    // 1) plain dispatch: the mod hook runs, nothing cancels; the OWNER runs the body when Invoke
//    reports 1 (Java: the woven method continues after the callback chain)
    MattiMixinResult result = MATTI_MIXIN_PASS;
    void *args[1] = {NULL};
    int runBody = LIBMATTI_MIXIN_Invoke(table, "matticraft::demo::tick", NULL, args, 1, &result);
    if (runBody) game_tick();
    printf("dispatch 1: body=%d result=%s (mod hook logged above)\n", runBody,
           result == MATTI_MIXIN_CANCEL ? "CANCEL" : "PASS");
    CHECK(runBody == 1 && result == MATTI_MIXIN_PASS && tickBodyRan,
          "mod hook ran without cancelling, target body ran");

    // 2) runtime hook at lower priority cancels at HEAD: body skipped
    tickBodyRan = 0;
    LIBMATTI_MIXIN_HookTable_AddHook(table, "matticraft::demo::tick", cancelling_hook, NULL,
                                     MATTI_MIXIN_AT_HEAD, MATTI_MIXIN_PRIORITY_DEFAULT - 1, "host");
    result = MATTI_MIXIN_PASS;
    runBody = LIBMATTI_MIXIN_Invoke(table, "matticraft::demo::tick", NULL, args, 1, &result);
    printf("dispatch 2: body=%d result=%s\n", runBody,
           result == MATTI_MIXIN_CANCEL ? "CANCEL" : "PASS");
    CHECK(runtimeHookRan && result == MATTI_MIXIN_CANCEL && runBody == 0 && !tickBodyRan,
          "runtime hook cancelled at HEAD, target body skipped");

    // 3) M2: patch the executable's own GOT and let a libc call prove the rewrite
    const char *symbols[] = {"puts"};
    size_t rewritten = LIBMATTI_MIXIN_GotPatch("/proc/self/exe", symbols, 1, puts_handler, NULL, "host");
    CHECK(rewritten == 1, "GOT slot rewritten");
    // The call must go through the PLT (jmp *GOT) - dlsym would bypass the patched slot.
    puts("this line must come back through the original puts");
    CHECK(m2Patched, "libc call landed on the mixin handler");

    LIBMATTI_MIXIN_HookTable_Free(table);
    printf(failures == 0 ? "ALL OK\n" : "%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}

// The .matti_mixins descriptor of the classmod fixture: one hook into the
// target the M4 harness executable declares, so the harness proves collection
// from a real mod .so (the examplemod build does the same in the real run).
#include "libmatti/matti_mixin.h"

#include <stdio.h>

static void classmod_mixin(MattiMixinCallbackInfo *info, void *userdata)
{
    (void) userdata;
    printf("classmod: mixin into %s (%zu argument(s))\n", info->target, info->argCount);
}

MATTI_MIXIN("classmod", "matticraft::demo::tick", classmod_mixin, NULL, MATTI_MIXIN_AT_HEAD,
            MATTI_MIXIN_PRIORITY_DEFAULT)

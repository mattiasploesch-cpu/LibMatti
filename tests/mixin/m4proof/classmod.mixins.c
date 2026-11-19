



#include "libmatti/matti_mixin.h"

#include <stdio.h>

static void classmod_mixin(MattiMixinCallbackInfo *info, void *userdata)
{
    (void) userdata;
    printf("classmod: mixin into %s (%zu argument(s))\n", info->target, info->argCount);
}

MATTI_MIXIN("classmod", "matticraft::demo::tick", classmod_mixin, NULL, MATTI_MIXIN_AT_HEAD,
            MATTI_MIXIN_PRIORITY_DEFAULT)

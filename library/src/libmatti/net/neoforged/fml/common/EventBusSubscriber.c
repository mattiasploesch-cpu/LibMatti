// Port of net.neoforged.fml.common.EventBusSubscriber.

#include "libmatti/net/neoforged/fml/common/EventBusSubscriber.h"

// Java: value() default { Dist.CLIENT, Dist.DEDICATED_SERVER }
static const LIBMATTI_DIST_Dist DEFAULT_SIDES[2] = {LIBMATTI_DIST_CLIENT, LIBMATTI_DIST_DEDICATED_SERVER};

LIBMATTI_FML_EventBusSubscriber LIBMATTI_FML_EventBusSubscriber_Defaults(void)
{
    LIBMATTI_FML_EventBusSubscriber subscriber = {DEFAULT_SIDES, 2, ""};
    return subscriber;
}

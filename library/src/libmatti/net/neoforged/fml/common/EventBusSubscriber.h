// Port of net.neoforged.fml.common.EventBusSubscriber.
// Java reads the annotation off a class through reflection (@Retention(RUNTIME) @Target(TYPE));
// C has neither annotations nor reflection, so the port passes the annotation's values as a value.
// Java defaults: value = { Dist.CLIENT, Dist.DEDICATED_SERVER }, modid = "".

#ifndef MATTICRAFT_FML_COMMON_EVENTBUSSUBSCRIBER_H
#define MATTICRAFT_FML_COMMON_EVENTBUSSUBSCRIBER_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"

#include <stddef.h>

// Java: public @interface EventBusSubscriber
typedef struct
{
    // Java: Dist[] value() default { Dist.CLIENT, Dist.DEDICATED_SERVER };
    const LIBMATTI_DIST_Dist *value;
    size_t valueCount;
    // Java: String modid() default "";
    const char *modid;
} LIBMATTI_FML_EventBusSubscriber;

// Java: the annotation defaults
LIBMATTI_FML_EventBusSubscriber LIBMATTI_FML_EventBusSubscriber_Defaults(void);

#endif //MATTICRAFT_FML_COMMON_EVENTBUSSUBSCRIBER_H







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

#endif

// Port of net.neoforged.bus.BusBuilderImpl.

#ifndef MATTICRAFT_BUS_BUSBUILDERIMPL_H
#define MATTICRAFT_BUS_BUSBUILDERIMPL_H

#include "libmatti/net/neoforged/bus/api/BusBuilder.h"

typedef struct LIBMATTI_BUS_BusBuilderImpl LIBMATTI_BUS_BusBuilderImpl;

struct LIBMATTI_BUS_BusBuilderImpl
{
    // Java: public final class BusBuilderImpl implements BusBuilder
    LIBMATTI_BUS_BusBuilder base;
    // Java: IEventExceptionHandler exceptionHandler; - null until it is set
    LIBMATTI_BUS_IEventExceptionHandler exceptionHandler;
    // Java: boolean startShutdown = false;
    int startShutdown;
    // Java: boolean checkTypesOnDispatch = false;
    int checkTypesOnDispatch;
    // Java: IEventClassChecker classChecker = eventClass -> {};
    LIBMATTI_BUS_IEventClassChecker classChecker;
    // Java: boolean allowPerPhasePost = false;
    int allowPerPhasePost;
};

// Java: BusBuilderImpl() - called by BusBuilder.builder()
LIBMATTI_BUS_BusBuilderImpl *LIBMATTI_BUS_BusBuilderImpl_New(void);
void LIBMATTI_BUS_BusBuilderImpl_Free(LIBMATTI_BUS_BusBuilderImpl *impl);

#endif //MATTICRAFT_BUS_BUSBUILDERIMPL_H

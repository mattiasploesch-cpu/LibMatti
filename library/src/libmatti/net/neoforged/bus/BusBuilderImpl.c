// Port of net.neoforged.bus.BusBuilderImpl.

#include "libmatti/net/neoforged/bus/BusBuilderImpl.h"

#include "libmatti/net/neoforged/bus/EventBus.h"

#include <stdlib.h>

// Java: IEventClassChecker classChecker = eventClass -> {};
static void default_class_check(void *self, LIBMATTI_JL_Class *eventClass)
{
    (void)self;
    (void)eventClass;
}

static LIBMATTI_BUS_BusBuilder *impl_set_exception_handler(void *self, LIBMATTI_BUS_IEventExceptionHandler handler)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = self;
    impl->exceptionHandler = handler;
    return &impl->base;
}

static LIBMATTI_BUS_BusBuilder *impl_start_shutdown(void *self)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = self;
    impl->startShutdown = 1;
    return &impl->base;
}

static LIBMATTI_BUS_BusBuilder *impl_check_types_on_dispatch(void *self)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = self;
    impl->checkTypesOnDispatch = 1;
    return &impl->base;
}

static LIBMATTI_BUS_BusBuilder *impl_class_checker(void *self, LIBMATTI_BUS_IEventClassChecker checker)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = self;
    impl->classChecker = checker;
    return &impl->base;
}

// Java: the markerType default method takes the BusBuilder, not the implementation
static LIBMATTI_BUS_BusBuilder *impl_marker_type(void *self, LIBMATTI_JL_Class *markerInterface)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = self;
    return LIBMATTI_BUS_BusBuilder_MarkerType(&impl->base, markerInterface);
}

static LIBMATTI_BUS_BusBuilder *impl_allow_per_phase_post(void *self)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = self;
    impl->allowPerPhasePost = 1;
    return &impl->base;
}

// Java: public IEventBus build() { return new EventBus(this); }
static LIBMATTI_BUS_IEventBus *impl_build(void *self)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = self;
    return &LIBMATTI_BUS_EventBus_New(impl)->busBase;
}

// Java: BusBuilderImpl()
LIBMATTI_BUS_BusBuilderImpl *LIBMATTI_BUS_BusBuilderImpl_New(void)
{
    LIBMATTI_BUS_BusBuilderImpl *impl = calloc(1, sizeof(LIBMATTI_BUS_BusBuilderImpl));

    impl->base.self = impl;
    impl->base.setExceptionHandler = impl_set_exception_handler;
    impl->base.startShutdown = impl_start_shutdown;
    impl->base.checkTypesOnDispatch = impl_check_types_on_dispatch;
    // Java: markerType is a default method - it calls back into classChecker
    impl->base.markerType = impl_marker_type;
    impl->base.classChecker = impl_class_checker;
    impl->base.allowPerPhasePost = impl_allow_per_phase_post;
    impl->base.build = impl_build;

    impl->classChecker.check = default_class_check;
    return impl;
}

void LIBMATTI_BUS_BusBuilderImpl_Free(LIBMATTI_BUS_BusBuilderImpl *impl)
{
    free(impl);
}

// Java: static BusBuilder builder() { return new BusBuilderImpl(); }
LIBMATTI_BUS_BusBuilder *LIBMATTI_BUS_BusBuilder_Builder(void)
{
    return &LIBMATTI_BUS_BusBuilderImpl_New()->base;
}

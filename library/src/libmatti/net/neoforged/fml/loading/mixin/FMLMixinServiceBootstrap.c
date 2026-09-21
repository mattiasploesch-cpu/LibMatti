// Port of net.neoforged.fml.loading.mixin.FMLMixinServiceBootstrap.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinServiceBootstrap.h"

// Java: @Override public String getName() { return "fml"; }
static const char *bootstrap_get_name(void *self)
{
    (void) self;
    return "fml";
}

// Java: @Override public String getServiceClassName() { return FMLMixinService.class.getName(); }
static const char *bootstrap_get_service_class_name(void *self)
{
    (void) self;
    return "net.neoforged.fml.loading.mixin.FMLMixinService";
}

// Java: @Override public void bootstrap() {}
static void bootstrap_bootstrap(void *self)
{
    (void) self;
}

static LIBMATTI_SP_IMixinServiceBootstrap theBootstrap = {
    NULL,
    bootstrap_get_name,
    bootstrap_get_service_class_name,
    bootstrap_bootstrap,
};

// Java: public class FMLMixinServiceBootstrap implements IMixinServiceBootstrap
LIBMATTI_SP_IMixinServiceBootstrap *LIBMATTI_FML_FMLMixinServiceBootstrap_Instance(void)
{
    theBootstrap.self = &theBootstrap;
    return &theBootstrap;
}

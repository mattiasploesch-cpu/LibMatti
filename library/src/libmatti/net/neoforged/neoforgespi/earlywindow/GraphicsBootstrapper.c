#include "libmatti/net/neoforged/neoforgespi/earlywindow/GraphicsBootstrapper.h"

// Java: public String name()
const char *LIBMATTI_NEOFORGESPI_GraphicsBootstrapper_Name(
    const LIBMATTI_NEOFORGESPI_GraphicsBootstrapper *bootstrapper)
{
    return bootstrapper->name(bootstrapper->self);
}

// Java: public void bootstrap(String[] arguments)
void LIBMATTI_NEOFORGESPI_GraphicsBootstrapper_Bootstrap(
    const LIBMATTI_NEOFORGESPI_GraphicsBootstrapper *bootstrapper, const char *const *arguments, size_t argumentCount)
{
    bootstrapper->bootstrap(bootstrapper->self, arguments, argumentCount);
}

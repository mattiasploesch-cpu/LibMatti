// Port of net.neoforged.neoforgespi.earlywindow.GraphicsBootstrapper.

#ifndef MATTICRAFT_NEOFORGESPI_EARLYWINDOW_GRAPHICSBOOTSTRAPPER_H
#define MATTICRAFT_NEOFORGESPI_EARLYWINDOW_GRAPHICSBOOTSTRAPPER_H

#include <stddef.h>

// Java: public interface GraphicsBootstrapper
typedef struct LIBMATTI_NEOFORGESPI_GraphicsBootstrapper LIBMATTI_NEOFORGESPI_GraphicsBootstrapper;

struct LIBMATTI_NEOFORGESPI_GraphicsBootstrapper
{
    void *self;

    // Java: String name()
    const char *(*name)(void *self);
    // Java: void bootstrap(String[] arguments)
    void (*bootstrap)(void *self, const char *const *arguments, size_t argumentCount);
};

// Java: public String name()
const char *LIBMATTI_NEOFORGESPI_GraphicsBootstrapper_Name(const LIBMATTI_NEOFORGESPI_GraphicsBootstrapper *bootstrapper);
// Java: public void bootstrap(String[] arguments)
void LIBMATTI_NEOFORGESPI_GraphicsBootstrapper_Bootstrap(const LIBMATTI_NEOFORGESPI_GraphicsBootstrapper *bootstrapper,
                                                         const char *const *arguments, size_t argumentCount);

#endif //MATTICRAFT_NEOFORGESPI_EARLYWINDOW_GRAPHICSBOOTSTRAPPER_H

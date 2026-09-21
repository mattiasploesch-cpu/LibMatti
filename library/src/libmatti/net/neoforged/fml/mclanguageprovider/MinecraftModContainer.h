// Port of net.neoforged.fml.mclanguageprovider.MinecraftModContainer.

#ifndef MATTICRAFT_FML_MCLANGUAGEPROVIDER_MINECRAFTMODCONTAINER_H
#define MATTICRAFT_FML_MCLANGUAGEPROVIDER_MINECRAFTMODCONTAINER_H

#include "libmatti/net/neoforged/fml/ModContainer.h"

// Java: public class MinecraftModContainer extends ModContainer
typedef struct LIBMATTI_FML_MinecraftModContainer LIBMATTI_FML_MinecraftModContainer;

struct LIBMATTI_FML_MinecraftModContainer
{
    // Java: extends ModContainer
    LIBMATTI_FML_ModContainer base;
};

// Java: public MinecraftModContainer(IModInfo info)
LIBMATTI_FML_MinecraftModContainer *LIBMATTI_FML_MinecraftModContainer_New(LIBMATTI_NEOFORGESPI_IModInfo *info);

#endif //MATTICRAFT_FML_MCLANGUAGEPROVIDER_MINECRAFTMODCONTAINER_H

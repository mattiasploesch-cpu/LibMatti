// Port of net.neoforged.fml.mclanguageprovider.MinecraftModLanguageProvider.

#ifndef MATTICRAFT_FML_MCLANGUAGEPROVIDER_MINECRAFTMODLANGUAGEPROVIDER_H
#define MATTICRAFT_FML_MCLANGUAGEPROVIDER_MINECRAFTMODLANGUAGEPROVIDER_H

#include "libmatti/net/neoforged/neoforgespi/language/IModLanguageLoader.h"

// Java: public class MinecraftModLanguageProvider extends BuiltInLanguageLoader
// Java: public String name() { return "minecraft"; }
#define LIBMATTI_FML_MinecraftModLanguageProvider_NAME "minecraft"

// Java: the instance used as an IModLanguageLoader
LIBMATTI_NEOFORGESPI_IModLanguageLoader *LIBMATTI_FML_MinecraftModLanguageProvider_AsLanguageLoader(void);

#endif //MATTICRAFT_FML_MCLANGUAGEPROVIDER_MINECRAFTMODLANGUAGEPROVIDER_H

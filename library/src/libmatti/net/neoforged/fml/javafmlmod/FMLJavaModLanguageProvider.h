// Port of net.neoforged.fml.javafmlmod.FMLJavaModLanguageProvider.
// Java streams over the scan data's @Mod annotations; the port keeps the same filters and sort.

#ifndef MATTICRAFT_FML_JAVAFMLMOD_FMLJAVAMODLANGUAGEPROVIDER_H
#define MATTICRAFT_FML_JAVAFMLMOD_FMLJAVAMODLANGUAGEPROVIDER_H

#include "libmatti/net/neoforged/neoforgespi/language/IModLanguageLoader.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"

// Java: public static final String NAME = "javafml"
#define LIBMATTI_FML_FMLJavaModLanguageProvider_NAME "javafml"

// Java: the instance used as an IModLanguageLoader
LIBMATTI_NEOFORGESPI_IModLanguageLoader *LIBMATTI_FML_FMLJavaModLanguageProvider_AsLanguageLoader(void);

// Java: private static List<String> getDepends(ModFileScanData.AnnotationData data)
// The port's annotation values are strings, so the array is the same delimited form
// AutomaticEventSubscriber.getSides uses.
size_t LIBMATTI_FML_FMLJavaModLanguageProvider_GetDepends(const char *data, const char **depends, size_t capacity);

#endif //MATTICRAFT_FML_JAVAFMLMOD_FMLJAVAMODLANGUAGEPROVIDER_H

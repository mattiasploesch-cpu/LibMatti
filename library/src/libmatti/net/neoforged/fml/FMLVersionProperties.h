// Port of net.neoforged.fml.FMLVersionProperties.
// Java: final class FMLVersionProperties { static final String VERSION = "<version>"; }
// The class is generated at build time by fmlbuild.GenerateVersionClass from
// the gradle project version; the C port takes it from a compile definition that
// CMake fills from a `git describe` of the cloned FML repository (see library/CMakeLists.txt).

#ifndef MATTICRAFT_FML_FMLVERSIONPROPERTIES_H
#define MATTICRAFT_FML_FMLVERSIONPROPERTIES_H

// Java: static final String VERSION
#ifndef LIBMATTI_FML_VERSION
#define LIBMATTI_FML_VERSION "26.1.0"
#endif

// Java: static final String VERSION
extern const char *const LIBMATTI_FML_FMLVersionProperties_VERSION;

#endif //MATTICRAFT_FML_FMLVERSIONPROPERTIES_H

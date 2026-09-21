// Port of net.neoforged.fml.loading.BuiltInLanguageLoader.
// Java: public abstract class BuiltInLanguageLoader implements IModLanguageLoader, providing version().
// Java reads the version from the class's code source (JarVersionLookupHandler) and falls back to
// FMLVersion.getVersion() for a directory; the C port has no per-class code source, so the built-in
// loaders report the loader version.

#ifndef MATTICRAFT_FML_LOADING_BUILTINLANGUAGELOADER_H
#define MATTICRAFT_FML_LOADING_BUILTINLANGUAGELOADER_H

// Java: public String version()
const char *LIBMATTI_FML_BuiltInLanguageLoader_Version(void);

#endif //MATTICRAFT_FML_LOADING_BUILTINLANGUAGELOADER_H

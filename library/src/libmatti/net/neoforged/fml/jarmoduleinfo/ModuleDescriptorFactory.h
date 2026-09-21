// Port of net.neoforged.fml.jarmoduleinfo.ModuleDescriptorFactory.
// Java uses java.util.regex.Pattern for the JDK ModulePath sanitization rules; the
// port implements the same matches with explicit scans.

#ifndef MATTICRAFT_FML_JARMODULEINFO_MODULEDESCRIPTORFACTORY_H
#define MATTICRAFT_FML_JARMODULEINFO_MODULEDESCRIPTORFACTORY_H

#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/NameAndVersion.h"

// Java: static NameAndVersion computeNameAndVersion(Path path)
LIBMATTI_FML_NameAndVersion *LIBMATTI_FML_ModuleDescriptorFactory_ComputeNameAndVersion(const char *path);

// Java: public static void scanAutomaticModule(JarContents jar, ModuleDescriptor.Builder builder, String... excludedRootDirectories)
void LIBMATTI_FML_ModuleDescriptorFactory_ScanAutomaticModule(LIBMATTI_FML_JarContents *jar,
                                                             LIBMATTI_JL_ModuleDescriptor *builder,
                                                             const char **excludedRootDirectories,
                                                             size_t excludedRootDirectoryCount);

// Java: public static Set<String> scanModulePackages(JarContents jar) - returns a new array, caller frees
char **LIBMATTI_FML_ModuleDescriptorFactory_ScanModulePackages(LIBMATTI_FML_JarContents *jar, size_t *count);

// Java: private static String cleanModuleName(String mn) - returns a new string
char *LIBMATTI_FML_ModuleDescriptorFactory_CleanModuleName(const char *name);

#endif //MATTICRAFT_FML_JARMODULEINFO_MODULEDESCRIPTORFACTORY_H

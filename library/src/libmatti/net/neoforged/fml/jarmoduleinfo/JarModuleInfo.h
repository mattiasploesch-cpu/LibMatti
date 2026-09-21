// Port of net.neoforged.fml.jarmoduleinfo.JarModuleInfo.
// ModuleDescriptor.read exists (ModuleDescriptorRead.c parses the compiled
// module-info.class), implemented by ModuleJarModuleInfo.

#ifndef MATTICRAFT_FML_JARMODULEINFO_JARMODULEINFO_H
#define MATTICRAFT_FML_JARMODULEINFO_JARMODULEINFO_H

#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"

#include <stddef.h>

// Java: public interface JarModuleInfo
typedef struct LIBMATTI_FML_JarModuleInfo LIBMATTI_FML_JarModuleInfo;

struct LIBMATTI_FML_JarModuleInfo
{
    void *self;

    // Java: String name()
    const char *(*name)(void *self);
    // Java: @Nullable String version()
    const char *(*version)(void *self);
    // Java: ModuleDescriptor createDescriptor(JarContents contents)
    LIBMATTI_JL_ModuleDescriptor *(*createDescriptor)(void *self, LIBMATTI_FML_JarContents *contents);
    // Java: the module info is owned by ModFile and collected by the GC; the port releases it explicitly
    void (*free)(void *self);
};

// Java: static JarModuleInfo from(JarContents jar)
LIBMATTI_FML_JarModuleInfo *LIBMATTI_FML_JarModuleInfo_From(LIBMATTI_FML_JarContents *jar);

// Java: static void scanAutomaticModule(JarContents jar, ModuleDescriptor.Builder builder, String... excludedRootDirectories)
void LIBMATTI_FML_JarModuleInfo_ScanAutomaticModule(LIBMATTI_FML_JarContents *jar,
                                                    LIBMATTI_JL_ModuleDescriptor *builder,
                                                    const char **excludedRootDirectories,
                                                    size_t excludedRootDirectoryCount);
// Java: static Set<String> scanModulePackages(JarContents jar)
char **LIBMATTI_FML_JarModuleInfo_ScanModulePackages(LIBMATTI_FML_JarContents *jar, size_t *count);

// Java: public String name()
const char *LIBMATTI_FML_JarModuleInfo_Name(const LIBMATTI_FML_JarModuleInfo *moduleInfo);
// Java: public @Nullable String version()
const char *LIBMATTI_FML_JarModuleInfo_Version(const LIBMATTI_FML_JarModuleInfo *moduleInfo);
// Java: public ModuleDescriptor createDescriptor(JarContents contents)
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_FML_JarModuleInfo_CreateDescriptor(const LIBMATTI_FML_JarModuleInfo *moduleInfo,
                                                                          LIBMATTI_FML_JarContents *contents);
// Java: the GC - releases the module info
void LIBMATTI_FML_JarModuleInfo_Free(LIBMATTI_FML_JarModuleInfo *moduleInfo);

#endif //MATTICRAFT_FML_JARMODULEINFO_JARMODULEINFO_H

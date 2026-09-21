// Port of net.neoforged.fml.classloading.JarContentsModule.

#ifndef MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULE_H
#define MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULE_H

#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"

// Java: public record JarContentsModule(JarContents contents, ModuleDescriptor moduleDescriptor)
typedef struct
{
    LIBMATTI_FML_JarContents *contents;
    LIBMATTI_JL_ModuleDescriptor *moduleDescriptor;
} LIBMATTI_FML_JarContentsModule;

// Java: public JarContentsModule(JarContents contents) { this(contents, JarModuleInfo.from(contents).createDescriptor(contents)); }
LIBMATTI_FML_JarContentsModule *LIBMATTI_FML_JarContentsModule_New(const LIBMATTI_FML_JarContents *contents);
// Java: public JarContentsModule(JarContents contents, ModuleDescriptor moduleDescriptor)
LIBMATTI_FML_JarContentsModule *LIBMATTI_FML_JarContentsModule_NewWithDescriptor(
    const LIBMATTI_FML_JarContents *contents, LIBMATTI_JL_ModuleDescriptor *moduleDescriptor);
// Java: public String moduleName() { return moduleDescriptor.name(); }
const char *LIBMATTI_FML_JarContentsModule_ModuleName(const LIBMATTI_FML_JarContentsModule *module);

#endif //MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULE_H

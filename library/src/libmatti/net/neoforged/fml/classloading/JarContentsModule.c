#include "libmatti/net/neoforged/fml/classloading/JarContentsModule.h"

#include "libmatti/net/neoforged/fml/jarmoduleinfo/JarModuleInfo.h"

#include <stdlib.h>

// Java: public JarContentsModule(JarContents contents)
LIBMATTI_FML_JarContentsModule *LIBMATTI_FML_JarContentsModule_New(const LIBMATTI_FML_JarContents *contents)
{
    // Java: this(contents, JarModuleInfo.from(contents).createDescriptor(contents))
    LIBMATTI_FML_JarModuleInfo *moduleInfo = LIBMATTI_FML_JarModuleInfo_From((LIBMATTI_FML_JarContents *)contents);
    LIBMATTI_JL_ModuleDescriptor *descriptor =
        LIBMATTI_FML_JarModuleInfo_CreateDescriptor(moduleInfo, (LIBMATTI_FML_JarContents *)contents);
    LIBMATTI_FML_JarModuleInfo_Free(moduleInfo);

    return LIBMATTI_FML_JarContentsModule_NewWithDescriptor(contents, descriptor);
}

// Java: public JarContentsModule(JarContents contents, ModuleDescriptor moduleDescriptor)
LIBMATTI_FML_JarContentsModule *LIBMATTI_FML_JarContentsModule_NewWithDescriptor(
    const LIBMATTI_FML_JarContents *contents, LIBMATTI_JL_ModuleDescriptor *moduleDescriptor)
{
    LIBMATTI_FML_JarContentsModule *module = calloc(1, sizeof(LIBMATTI_FML_JarContentsModule));
    module->contents = (LIBMATTI_FML_JarContents *)contents;
    module->moduleDescriptor = moduleDescriptor;
    return module;
}

// Java: public String moduleName() { return moduleDescriptor.name(); }
const char *LIBMATTI_FML_JarContentsModule_ModuleName(const LIBMATTI_FML_JarContentsModule *module)
{
    return LIBMATTI_JL_ModuleDescriptor_Name(module->moduleDescriptor);
}

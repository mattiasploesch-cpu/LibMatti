// Port of net.neoforged.fml.jarmoduleinfo.ModuleJarModuleInfo.
// The class-file parser for the compiled module-info.class is ModuleDescriptorRead.c;
// the ByteBuffer port backs the parsing.

#ifndef MATTICRAFT_FML_JARMODULEINFO_MODULEJARMODULEINFO_H
#define MATTICRAFT_FML_JARMODULEINFO_MODULEJARMODULEINFO_H

#include "libmatti/net/neoforged/fml/jarmoduleinfo/JarModuleInfo.h"

// Java: public ModuleJarModuleInfo(JarResource moduleInfo)
LIBMATTI_FML_JarModuleInfo *LIBMATTI_FML_ModuleJarModuleInfo_New(LIBMATTI_FML_JarResource *moduleInfo);

#endif //MATTICRAFT_FML_JARMODULEINFO_MODULEJARMODULEINFO_H

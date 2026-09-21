// Port of net.neoforged.fml.jarcontents.JarFileContents.
// In Java this wraps the JDK ZipFile; here the artifact is an ELF shared object
// and the entries come from the ELF sections (see bsl/sjh/niofs/union).

#ifndef MATTICRAFT_FML_JARCONTENTS_JARFILECONTENTS_H
#define MATTICRAFT_FML_JARCONTENTS_JARFILECONTENTS_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"

// Java: public JarFileContents(Path path) throws IOException - NULL on failure
LIBMATTI_FML_JarContents *LIBMATTI_FML_JarFileContents_New(const char *path);

#endif //MATTICRAFT_FML_JARCONTENTS_JARFILECONTENTS_H

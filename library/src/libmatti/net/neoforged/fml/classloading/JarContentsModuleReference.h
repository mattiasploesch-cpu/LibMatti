// Port of net.neoforged.fml.classloading.JarContentsModuleReference.

#ifndef MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULEREFERENCE_H
#define MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULEREFERENCE_H

#include "libmatti/java/lang/module/ModuleReference.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"

// Java: static class JarContentsModuleReader implements ModuleReader (defined in JarContentsModuleReference.c)
typedef struct LIBMATTI_FML_JarContentsModuleReader LIBMATTI_FML_JarContentsModuleReader;

// Java: final class JarContentsModuleReference extends ModuleReference
typedef struct
{
    LIBMATTI_JL_ModuleReference base; // Java: super(descriptor, getModuleLocation(contents))
    LIBMATTI_FML_JarContents *contents;
} LIBMATTI_FML_JarContentsModuleReference;

// Java: JarContentsModuleReference(ModuleDescriptor descriptor, JarContents contents)
LIBMATTI_FML_JarContentsModuleReference *LIBMATTI_FML_JarContentsModuleReference_New(
    LIBMATTI_JL_ModuleDescriptor *descriptor, LIBMATTI_FML_JarContents *contents);

// Java: private static URI getModuleLocation(JarContents contents)
LIBMATTI_JN_URI *LIBMATTI_FML_JarContentsModuleReference_GetModuleLocation(LIBMATTI_FML_JarContents *contents);

// Java: public ModuleReader open() { return new JarContentsModuleReader(contents); }
LIBMATTI_FML_JarContentsModuleReader *LIBMATTI_FML_JarContentsModuleReference_Open(
    const LIBMATTI_FML_JarContentsModuleReference *reference);

// Java: JarContentsModuleReader(JarContents contents)
LIBMATTI_FML_JarContentsModuleReader *LIBMATTI_FML_JarContentsModuleReader_New(LIBMATTI_FML_JarContents *contents);
// Java: public Optional<InputStream> open(String name) - NULL = empty
unsigned char *LIBMATTI_FML_JarContentsModuleReader_OpenFile(const LIBMATTI_FML_JarContentsModuleReader *reader,
                                                            const char *name, size_t *outLength);
// Java: public Optional<URI> find(String name) - NULL = empty
LIBMATTI_JN_URI *LIBMATTI_FML_JarContentsModuleReader_Find(const LIBMATTI_FML_JarContentsModuleReader *reader,
                                                          const char *name);
// Java: public Stream<String> list() - returns a new array, caller frees
char **LIBMATTI_FML_JarContentsModuleReader_List(const LIBMATTI_FML_JarContentsModuleReader *reader, size_t *count);
// Java: public void close() {}
void LIBMATTI_FML_JarContentsModuleReader_Close(LIBMATTI_FML_JarContentsModuleReader *reader);
// Java: public String toString() { return contents.toString(); }
char *LIBMATTI_FML_JarContentsModuleReader_ToString(const LIBMATTI_FML_JarContentsModuleReader *reader);

#endif //MATTICRAFT_FML_CLASSLOADING_JARCONTENTSMODULEREFERENCE_H

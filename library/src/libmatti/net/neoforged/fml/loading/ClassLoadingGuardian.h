// Port of net.neoforged.fml.loading.ClassLoadingGuardian.
// TODO: org.objectweb.asm (ClassWriter, MethodVisitor, Opcodes, Type) - the generated
//       self-destructing class is Java bytecode, which has no meaning in the C port.
// TODO: org.spongepowered.asm.util.Constants - the generated <clinit> name.
// TODO: net.neoforged.fml.loading.moddiscovery.ModFile - the constructor takes the
//       module descriptor packages of the game content until moddiscovery is ported.

#ifndef MATTICRAFT_FML_LOADING_CLASSLOADINGGUARDIAN_H
#define MATTICRAFT_FML_LOADING_CLASSLOADINGGUARDIAN_H

#include "libmatti/java/lang/instrument/Instrumentation.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_ClassLoadingGuardian LIBMATTI_FML_ClassLoadingGuardian;

// Java: public ClassLoadingGuardian(Instrumentation instrumentation, List<ModFile> gameContent)
LIBMATTI_FML_ClassLoadingGuardian *LIBMATTI_FML_ClassLoadingGuardian_New(
    LIBMATTI_JLI_Instrumentation *instrumentation, const char *const *packages, size_t packageCount);

// Java: public static void fail()
void LIBMATTI_FML_ClassLoadingGuardian_Fail(void);

// Java: public void setAllowedClassLoader(ClassLoader allowedClassLoader)
void LIBMATTI_FML_ClassLoadingGuardian_SetAllowedClassLoader(LIBMATTI_FML_ClassLoadingGuardian *guardian,
                                                             void *allowedClassLoader);

// Java: ClassFileTransformer.transform(...) - the port calls this from the class-loading path.
// Returns the replacement class bytes (Java: byte[]) or NULL when the class is allowed through.
unsigned char *LIBMATTI_FML_ClassLoadingGuardian_Transform(LIBMATTI_FML_ClassLoadingGuardian *guardian,
                                                           void *loader, const char *className,
                                                           size_t *byteCount);

// Java: public void close()
void LIBMATTI_FML_ClassLoadingGuardian_Close(LIBMATTI_FML_ClassLoadingGuardian *guardian);

// the port frees the guardian struct itself (Java has no explicit free)
void LIBMATTI_FML_ClassLoadingGuardian_Free(LIBMATTI_FML_ClassLoadingGuardian *guardian);

#endif //MATTICRAFT_FML_LOADING_CLASSLOADINGGUARDIAN_H

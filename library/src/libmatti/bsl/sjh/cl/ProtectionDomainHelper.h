// Port of cpw.mods.cl.ProtectionDomainHelper.

#ifndef MATTICRAFT_PROTECTIONDOMAINHELPER_H
#define MATTICRAFT_PROTECTIONDOMAINHELPER_H

#include "libmatti/java/net/URI.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stddef.h>

// TODO: java.security.CodeSource (external)
typedef struct LIBMATTI_CL_CodeSource LIBMATTI_CL_CodeSource;
// TODO: java.security.ProtectionDomain (external)
typedef struct LIBMATTI_CL_ProtectionDomain LIBMATTI_CL_ProtectionDomain;

// Java: Function<String, Attributes> trustedEntries
typedef const LIBMATTI_JU_Attributes *(*LIBMATTI_CL_TrustedEntries)(const char *path);
// Java: Function<String[], Package> definePackage
typedef void *(*LIBMATTI_CL_DefinePackage)(const char **args);
// Java: ClassLoader.getDefinedPackage(pname) - 1 if already defined
typedef int (*LIBMATTI_CL_GetDefinedPackage)(const char *pname);

// Java: CodeSource createCodeSource(URL url, CodeSigner[] signers) - cache computeIfAbsent
LIBMATTI_CL_CodeSource *LIBMATTI_CL_ProtectionDomainHelper_CreateCodeSource(LIBMATTI_JN_URI *url, void **signers);

// Java: ProtectionDomain createProtectionDomain(CodeSource, ClassLoader) - cache computeIfAbsent
//       Permissions: new AllPermission()
LIBMATTI_CL_ProtectionDomain *LIBMATTI_CL_ProtectionDomainHelper_CreateProtectionDomain(LIBMATTI_CL_CodeSource *codeSource);

// Java: trySetPackageModule(Package, Module) - JVM VarHandle reflection, no C equivalent
void LIBMATTI_CL_ProtectionDomainHelper_TrySetPackageModule(void *pkg, void *module);

// Java: Package tryDefinePackage(ClassLoader, String name, Manifest, trustedEntries, definePackage)
// Returns the definePackage result; NULL when the package is already defined (Java: returns existing package)
void *LIBMATTI_CL_ProtectionDomainHelper_TryDefinePackage(const char *name, const LIBMATTI_JU_Manifest *man,
                                                         LIBMATTI_CL_TrustedEntries trustedEntries,
                                                         LIBMATTI_CL_DefinePackage definePackage,
                                                         LIBMATTI_CL_GetDefinedPackage getDefinedPackage);

#endif //MATTICRAFT_PROTECTIONDOMAINHELPER_H
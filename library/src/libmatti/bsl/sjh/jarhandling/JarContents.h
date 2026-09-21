//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.JarContents.

#ifndef MATTICRAFT_JARCONTENTS_H
#define MATTICRAFT_JARCONTENTS_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/java/net/URI.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stddef.h>

// Java: interface JarContents (implemented by JarContentsImpl)
typedef struct LIBMATTI_JH_JarContents LIBMATTI_JH_JarContents;

// Java: static JarContents.of(Path fileOrFolder)
LIBMATTI_JH_JarContents *LIBMATTI_JH_JarContents_Of(const char *path);
// Java: static JarContents.of(Collection<Path>) - NULL on failure
LIBMATTI_JH_JarContents *LIBMATTI_JH_JarContents_OfPaths(const char **paths, size_t pathCount);

// Java: getPrimaryPath()
const char *LIBMATTI_JH_JarContents_GetPrimaryPath(const LIBMATTI_JH_JarContents *contents);
// Java: Optional<URI> findFile(String name) - 1 if found, *out is a new URI
int LIBMATTI_JH_JarContents_FindFile(const LIBMATTI_JH_JarContents *contents, const char *name, LIBMATTI_JN_URI **out);
// Java: byte[] getFileBytes(String path) - reads an entry through the union fs;
// NULL when the entry is absent. Caller frees, *outLength set.
unsigned char *LIBMATTI_JH_JarContents_OpenFile(const LIBMATTI_JH_JarContents *contents, const char *name,
                                                size_t *outLength);
// Java: getManifest()
LIBMATTI_JU_Manifest *LIBMATTI_JH_JarContents_GetManifest(const LIBMATTI_JH_JarContents *contents);
// Java: Set<String> getPackages() - returns the internal cache, do not free
char **LIBMATTI_JH_JarContents_GetPackages(const LIBMATTI_JH_JarContents *contents, size_t *count);
// Java: Set<String> getPackagesExcluding(String... excludedRootPackages)
char **LIBMATTI_JH_JarContents_GetPackagesExcluding(const LIBMATTI_JH_JarContents *contents,
                                                    char **excludedRootPackages, size_t excludedCount, size_t *count);
// Java: List<Provider> getMetaInfServices() - returns the internal cache, do not free
LIBMATTI_JH_Provider *LIBMATTI_JH_JarContents_GetMetaInfServices(const LIBMATTI_JH_JarContents *contents, size_t *count);
// Java: close()
void LIBMATTI_JH_JarContents_Close(LIBMATTI_JH_JarContents *contents);

#endif //MATTICRAFT_JARCONTENTS_H
//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.impl.Jar.

#ifndef MATTICRAFT_JAR_H
#define MATTICRAFT_JAR_H

#include "libmatti/bsl/sjh/jarhandling/JarMetadata.h"
#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/bsl/sjh/jarhandling/impl/JarContentsImpl.h"

typedef struct LIBMATTI_JH_Jar LIBMATTI_JH_Jar;

// Java: private record JarModuleDataProvider(Jar jar) implements ModuleDataProvider
typedef struct
{
    LIBMATTI_JH_ModuleDataProvider base; // Java: implements ModuleDataProvider
    LIBMATTI_JH_Jar *jar;
} LIBMATTI_JH_JarModuleDataProvider;

// Java: class Jar implements SecureJar
typedef struct LIBMATTI_JH_Jar
{
    LIBMATTI_JH_SecureJar base; // Java: implements SecureJar
    LIBMATTI_JH_JarContentsImpl *contents;
    LIBMATTI_JU_Manifest *manifest; // = contents->manifest
    // TODO: signingData (JarSigningData)
    LIBMATTI_UNION_UnionFileSystem *filesystem; // = contents->filesystem
    LIBMATTI_JH_JarModuleDataProvider moduleDataProvider;
    LIBMATTI_JH_JarMetadata *metadata;
} LIBMATTI_JH_Jar;

// Java: Jar(JarContentsImpl contents, JarMetadata metadata)
LIBMATTI_JH_Jar *LIBMATTI_JH_Jar_New(LIBMATTI_JH_JarContentsImpl *contents, LIBMATTI_JH_JarMetadata *metadata);

// Java: URI getURI() - the filesystem root URI (new, caller frees)
LIBMATTI_JN_URI *LIBMATTI_JH_Jar_GetURI(const LIBMATTI_JH_Jar *jar);
// Java: ModuleDescriptor computeDescriptor()
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JH_Jar_ComputeDescriptor(const LIBMATTI_JH_Jar *jar);
// Java: Optional<URI> findFile(String name) - 1 if found, *out is a new URI
int LIBMATTI_JH_Jar_FindFile(const LIBMATTI_JH_Jar *jar, const char *name, LIBMATTI_JN_URI **out);

#endif //MATTICRAFT_JAR_H
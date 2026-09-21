//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.VirtualJar.

#ifndef MATTICRAFT_VIRTUALJAR_H
#define MATTICRAFT_VIRTUALJAR_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/bsl/sjh/niofs/union/UnionFileSystem.h"
#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stddef.h>

typedef struct LIBMATTI_JH_VirtualJar LIBMATTI_JH_VirtualJar;

// Java: private class VirtualJarModuleDataProvider implements ModuleDataProvider
typedef struct
{
    LIBMATTI_JH_ModuleDataProvider base;
    LIBMATTI_JH_VirtualJar *jar;
} LIBMATTI_JH_VirtualJarModuleDataProvider;

// Java: final class VirtualJar implements SecureJar
// A SecureJar without actual files, but with defined packages.
struct LIBMATTI_JH_VirtualJar
{
    LIBMATTI_JH_SecureJar base;
    LIBMATTI_JL_ModuleDescriptor *moduleDescriptor;
    LIBMATTI_JH_VirtualJarModuleDataProvider moduleData;
    LIBMATTI_UNION_UnionFileSystem *dummyFileSystem;
    LIBMATTI_JU_Manifest *manifest; // new Manifest() - empty
};

// Java: VirtualJar(String name, Path referencePath, String... packages)
// NULL if the reference path does not exist (Java: IllegalArgumentException)
LIBMATTI_JH_VirtualJar *LIBMATTI_JH_VirtualJar_New(const char *name, const char *referencePath,
                                                   char **packages, size_t packageCount);

#endif //MATTICRAFT_VIRTUALJAR_H
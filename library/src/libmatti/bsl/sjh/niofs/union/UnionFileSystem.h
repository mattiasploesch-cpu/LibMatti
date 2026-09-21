//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.niofs.union.UnionFileSystem.
// In Java this delegates the actual archive reading to the JDK zip filesystem
// (FileSystems.newFileSystem); here the artifact is a .so and its ELF sections
// are read directly.
// Entry mapping: the section ".matti_manifest" is exposed as "META-INF/MANIFEST.MF",
// and every other section whose name is not a toolchain name (".text", ".rodata", ...)
// is exposed as the entry of exactly that path.

#ifndef MATTICRAFT_UNIONFILESYSTEM_H
#define MATTICRAFT_UNIONFILESYSTEM_H

#include "libmatti/bsl/sjh/niofs/union/UnionPathFilter.h"
#include "libmatti/java/net/URI.h"

#include <stddef.h>
#include <stdint.h>

typedef struct LIBMATTI_UNION_UnionFileSystemProvider LIBMATTI_UNION_UnionFileSystemProvider;
typedef struct LIBMATTI_UNION_UnionPath LIBMATTI_UNION_UnionPath;

// Java: EmbeddedFileSystemMetadata(Path path, FileSystem fs, SeekableByteChannel fsCh)
// The fsCh channel hack (JVM reflection) has no C equivalent.
typedef struct LIBMATTI_UNION_EmbeddedElf LIBMATTI_UNION_EmbeddedElf;

// Java: BasicFileAttributes (minimal subset used by the port)
typedef struct
{
    int isDirectory;
    int isRegularFile;
    long size;
} LIBMATTI_UNION_BasicFileAttributes;

// Java: class UnionFileSystem extends FileSystem
typedef struct LIBMATTI_UNION_UnionFileSystem
{
    LIBMATTI_UNION_UnionFileSystemProvider *provider;
    char *key;
    char **basePaths; // reversed + exists-filtered input: later elements are first in search order
    size_t basePathCount;
    int lastElementIndex;
    LIBMATTI_UNION_UnionPathFilter *pathFilter; // NULL = no filter
    LIBMATTI_UNION_EmbeddedElf **embeddedElfs; // .so bases only
    size_t embeddedElfCount;
    LIBMATTI_UNION_UnionPath *root;
} LIBMATTI_UNION_UnionFileSystem;

// Java: constructor - NULL if a base path cannot be opened as a .so
LIBMATTI_UNION_UnionFileSystem *LIBMATTI_UNION_UnionFileSystem_New(LIBMATTI_UNION_UnionFileSystemProvider *provider,
                                                                   LIBMATTI_UNION_UnionPathFilter *pathFilter,
                                                                   const char *key, const char **paths, size_t pathCount);
// Java: close()
void LIBMATTI_UNION_UnionFileSystem_Close(LIBMATTI_UNION_UnionFileSystem *fileSystem);

// Java: getPrimaryPath() - the first of the original input paths
const char *LIBMATTI_UNION_UnionFileSystem_GetPrimaryPath(const LIBMATTI_UNION_UnionFileSystem *fileSystem);
// Java: getFilesystemFilter()
LIBMATTI_UNION_UnionPathFilter *LIBMATTI_UNION_UnionFileSystem_GetFilesystemFilter(const LIBMATTI_UNION_UnionFileSystem *fileSystem);
// Java: getKey() - returns a copy, caller frees
char *LIBMATTI_UNION_UnionFileSystem_GetKey(const LIBMATTI_UNION_UnionFileSystem *fileSystem);
// Java: getRoot()
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionFileSystem_GetRoot(const LIBMATTI_UNION_UnionFileSystem *fileSystem);
// Java: getPath(String first, String... more)
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionFileSystem_GetPath(const LIBMATTI_UNION_UnionFileSystem *fileSystem,
                                                                 const char **parts, size_t partCount);

// Java: exists(UnionPath)
int LIBMATTI_UNION_UnionFileSystem_Exists(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const LIBMATTI_UNION_UnionPath *path);
// Java: readAttributesIfExists(UnionPath, BasicFileAttributes) - 1 if found, 0 otherwise
int LIBMATTI_UNION_UnionFileSystem_ReadAttributesIfExists(const LIBMATTI_UNION_UnionFileSystem *fileSystem,
                                                          const LIBMATTI_UNION_UnionPath *path,
                                                          LIBMATTI_UNION_BasicFileAttributes *out);
// Java: buildInputStream / newReadByteChannel + readAllBytes - 1 on success
int LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(const LIBMATTI_UNION_UnionFileSystem *fileSystem,
                                                const LIBMATTI_UNION_UnionPath *path,
                                                unsigned char **outBytes, size_t *outLength);

// Java: newDirStream/walkFileTree - enumerates entries under dirRel across all base paths,
// applying the path filter, deduplicated. callback returns 1 to continue, 0 to stop.
int LIBMATTI_UNION_UnionFileSystem_Walk(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *dirRel, int recursive,
                                        int (*callback)(const char *relativePath, int isDirectory, void *userdata),
                                        void *userdata);

#endif //MATTICRAFT_UNIONFILESYSTEM_H
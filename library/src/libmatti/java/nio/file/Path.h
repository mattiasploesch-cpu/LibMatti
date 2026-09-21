// Port of java.nio.file.Path. The C port carries paths as plain strings; every
// function returns a newly allocated string that the caller frees.

#ifndef MATTICRAFT_JAVA_NIO_FILE_PATH_H
#define MATTICRAFT_JAVA_NIO_FILE_PATH_H

#include <stddef.h>

// Java: static Path of(String first, String... more)
char *LIBMATTI_JNF_Path_Of(const char *first, const char **more, size_t moreCount);
// Java: Path resolve(String other)
char *LIBMATTI_JNF_Path_Resolve(const char *path, const char *other);
// Java: Path toAbsolutePath()
char *LIBMATTI_JNF_Path_ToAbsolutePath(const char *path);
// Java: Path normalize()
char *LIBMATTI_JNF_Path_Normalize(const char *path);
// Java: Path getFileName() (NULL = empty path / root)
char *LIBMATTI_JNF_Path_GetFileName(const char *path);
// Java: Path getParent() (NULL = no parent)
char *LIBMATTI_JNF_Path_GetParent(const char *path);

#endif //MATTICRAFT_JAVA_NIO_FILE_PATH_H

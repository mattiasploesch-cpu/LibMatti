// Port of java.nio.file.Files (the operations the loader uses).

#ifndef MATTICRAFT_JAVA_NIO_FILE_FILES_H
#define MATTICRAFT_JAVA_NIO_FILE_FILES_H

#include <stddef.h>

// Java: static boolean exists(Path path, LinkOption... options)
int LIBMATTI_JNF_Files_Exists(const char *path);
// Java: static boolean isDirectory(Path path, LinkOption... options)
int LIBMATTI_JNF_Files_IsDirectory(const char *path);
// Java: static boolean isRegularFile(Path path, LinkOption... options)
int LIBMATTI_JNF_Files_IsRegularFile(const char *path);
// Java: static Path createDirectories(Path dir, FileAttribute<?>... attrs) - 0 on failure
int LIBMATTI_JNF_Files_CreateDirectories(const char *path);
// Java: static boolean deleteIfExists(Path path)
int LIBMATTI_JNF_Files_DeleteIfExists(const char *path);
// Java: static byte[] readAllBytes(Path path) - NULL on failure, *outLength set
unsigned char *LIBMATTI_JNF_Files_ReadAllBytes(const char *path, size_t *outLength);

// Java: enum FileVisitResult { CONTINUE, TERMINATE, SKIP_SUBTREE, SKIP_SIBLINGS }
typedef enum
{
    LIBMATTI_JNF_FileVisitResult_CONTINUE,
    LIBMATTI_JNF_FileVisitResult_TERMINATE,
    LIBMATTI_JNF_FileVisitResult_SKIP_SUBTREE,
    LIBMATTI_JNF_FileVisitResult_SKIP_SIBLINGS
} LIBMATTI_JNF_FileVisitResult;

// Java: class SimpleFileVisitor<T> implements FileVisitor<T>
typedef struct
{
    void *self;
    // Java: public FileVisitResult visitFile(T file, BasicFileAttributes attrs)
    LIBMATTI_JNF_FileVisitResult (*visitFile)(void *self, const char *file, int isRegularFile, long size);
} LIBMATTI_JNF_SimpleFileVisitor;

// Java: static Path walkFileTree(Path start, Set<FileVisitOption> options, int maxDepth, FileVisitor<? super Path> visitor)
int LIBMATTI_JNF_Files_WalkFileTree(const char *start, int followLinks, int maxDepth,
                                    LIBMATTI_JNF_SimpleFileVisitor *visitor);

// Java: static Stream<Path> list(Path dir) - the caller frees each entry and the array; NULL on failure
char **LIBMATTI_JNF_Files_List(const char *dir, size_t *count);

// Java: static Path write(Path path, byte[] bytes, OpenOption... options) - 0 on failure
int LIBMATTI_JNF_Files_Write(const char *path, const unsigned char *bytes, size_t length);
// Java: static Path writeString(Path path, CharSequence string)
int LIBMATTI_JNF_Files_WriteString(const char *path, const char *string);
// Java: static String readString(Path path) - NULL on failure, caller frees
char *LIBMATTI_JNF_Files_ReadString(const char *path);
// Java: static Path createFile(Path path) - 0 on failure
int LIBMATTI_JNF_Files_CreateFile(const char *path);
// Java: static Path createTempDirectory(String prefix) - NULL on failure, caller frees
char *LIBMATTI_JNF_Files_CreateTempDirectory(const char *prefix);
// Java: static Path createTempFile(String prefix, String suffix) - NULL on failure, caller frees
char *LIBMATTI_JNF_Files_CreateTempFile(const char *prefix, const char *suffix);
// Java: static long size(Path path) - -1 on failure
long long LIBMATTI_JNF_Files_Size(const char *path);
// Java: static boolean isRegularFile / exists are above; add notExists
int LIBMATTI_JNF_Files_NotExists(const char *path);
// Java: static void copy(Path source, Path target) - 0 on failure
int LIBMATTI_JNF_Files_Copy(const char *source, const char *target);
// Java: static Path move(Path source, Path target) - 0 on failure
int LIBMATTI_JNF_Files_Move(const char *source, const char *target);

#endif //MATTICRAFT_JAVA_NIO_FILE_FILES_H

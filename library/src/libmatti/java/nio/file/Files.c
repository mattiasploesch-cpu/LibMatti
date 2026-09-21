#include "libmatti/java/nio/file/Files.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

// Java: static boolean exists(Path path, LinkOption... options)
int LIBMATTI_JNF_Files_Exists(const char *path)
{
    struct stat status;
    return stat(path, &status) == 0;
}

// Java: static boolean isDirectory(Path path, LinkOption... options)
int LIBMATTI_JNF_Files_IsDirectory(const char *path)
{
    struct stat status;
    return stat(path, &status) == 0 && S_ISDIR(status.st_mode);
}

// Java: static boolean isRegularFile(Path path, LinkOption... options)
int LIBMATTI_JNF_Files_IsRegularFile(const char *path)
{
    struct stat status;
    return stat(path, &status) == 0 && S_ISREG(status.st_mode);
}

// Java: static Path createDirectories(Path dir, FileAttribute<?>... attrs)
int LIBMATTI_JNF_Files_CreateDirectories(const char *path)
{
    char *copy = strdup(path);
    int created = 1;

    for (char *cursor = copy + 1; *cursor != '\0'; cursor++)
    {
        if (*cursor != '/') continue;

        *cursor = '\0';
        if (mkdir(copy, 0755) != 0 && errno != EEXIST) created = 0;
        *cursor = '/';
    }

    if (mkdir(copy, 0755) != 0 && !LIBMATTI_JNF_Files_IsDirectory(copy)) created = 0;

    free(copy);
    return created;
}

// Java: static boolean deleteIfExists(Path path)
int LIBMATTI_JNF_Files_DeleteIfExists(const char *path)
{
    if (remove(path) == 0) return 1;
    return errno == ENOENT ? 0 : 0;
}

// Java: static byte[] readAllBytes(Path path)
unsigned char *LIBMATTI_JNF_Files_ReadAllBytes(const char *path, size_t *outLength)
{
    *outLength = 0;
    if (LIBMATTI_JNF_Files_IsDirectory(path)) return NULL;

    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length < 0)
    {
        fclose(file);
        return NULL;
    }

    unsigned char *bytes = malloc((size_t)length + 1);
    if (fread(bytes, 1, (size_t)length, file) != (size_t)length)
    {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);

    bytes[length] = '\0';
    *outLength = (size_t)length;
    return bytes;
}

// Java: static Path walkFileTree(Path start, Set<FileVisitOption> options, int maxDepth, FileVisitor visitor)
static int walkFileTree(const char *start, LIBMATTI_JNF_SimpleFileVisitor *visitor, int depth, int maxDepth)
{
    DIR *directory = opendir(start);
    if (directory == NULL)
        return 1;

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t length = strlen(start) + strlen(entry->d_name) + 2;
        char *child = malloc(length);
        snprintf(child, length, "%s/%s", start, entry->d_name);

        struct stat status;
        if (stat(child, &status) == 0)
        {
            if (S_ISDIR(status.st_mode))
            {
                if (depth < maxDepth && !walkFileTree(child, visitor, depth + 1, maxDepth))
                {
                    free(child);
                    closedir(directory);
                    return 0;
                }
            }
            else if (visitor->visitFile(visitor->self, child, S_ISREG(status.st_mode), (long) status.st_size) ==
                     LIBMATTI_JNF_FileVisitResult_TERMINATE)
            {
                free(child);
                closedir(directory);
                return 0;
            }
        }

        free(child);
    }

    closedir(directory);
    return 1;
}

// Java: static Path walkFileTree(Path start, Set<FileVisitOption> options, int maxDepth, FileVisitor<? super Path> visitor)
int LIBMATTI_JNF_Files_WalkFileTree(const char *start, int followLinks, int maxDepth,
                                    LIBMATTI_JNF_SimpleFileVisitor *visitor)
{
    (void) followLinks;
    return walkFileTree(start, visitor, 0, maxDepth);
}

// Java: static Stream<Path> list(Path dir)
char **LIBMATTI_JNF_Files_List(const char *dir, size_t *count)
{
    DIR *directory = opendir(dir);
    if (directory == NULL)
        return NULL;

    char **entries = NULL;
    *count = 0;

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t length = strlen(dir) + strlen(entry->d_name) + 2;
        char *child = malloc(length);
        snprintf(child, length, "%s/%s", dir, entry->d_name);

        entries = realloc(entries, sizeof(char *) * (*count + 1));
        entries[(*count)++] = child;
    }

    closedir(directory);
    return entries;
}

// Java: static Path write(Path path, byte[] bytes, OpenOption... options)
int LIBMATTI_JNF_Files_Write(const char *path, const unsigned char *bytes, size_t length)
{
    FILE *file = fopen(path, "wb");
    if (file == NULL)
        return 0;
    size_t written = fwrite(bytes, 1, length, file);
    fclose(file);
    return written == length;
}

// Java: static Path writeString(Path path, CharSequence string)
int LIBMATTI_JNF_Files_WriteString(const char *path, const char *string)
{
    return LIBMATTI_JNF_Files_Write(path, (const unsigned char *) string, strlen(string));
}

// Java: static String readString(Path path)
char *LIBMATTI_JNF_Files_ReadString(const char *path)
{
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_JNF_Files_ReadAllBytes(path, &length);
    if (bytes == NULL)
        return NULL;
    // Java: decoded UTF-8; the port's strings are UTF-8 already
    bytes[length] = '\0';
    return (char *) bytes;
}

// Java: static Path createFile(Path path)
int LIBMATTI_JNF_Files_CreateFile(const char *path)
{
    if (LIBMATTI_JNF_Files_Exists(path))
        return 0; // Java: FileAlreadyExistsException
    FILE *file = fopen(path, "wb");
    if (file == NULL)
        return 0;
    fclose(file);
    return 1;
}

// Java: static Path createTempDirectory(String prefix, FileAttribute<?>... attrs)
char *LIBMATTI_JNF_Files_CreateTempDirectory(const char *prefix)
{
    char tmpl[] = "/tmp/matti-XXXXXX";
    if (mkdtemp(tmpl) == NULL)
        return NULL;
    (void) prefix;
    return strdup(tmpl);
}

// Java: static Path createTempFile(String prefix, String suffix, FileAttribute<?>... attrs)
char *LIBMATTI_JNF_Files_CreateTempFile(const char *prefix, const char *suffix)
{
    const char *defaultPrefix = prefix != NULL ? prefix : "matti";
    const char *defaultSuffix = suffix != NULL ? suffix : ".tmp";
    char *tmpl = malloc(strlen(defaultPrefix) + strlen(defaultSuffix) + 13);
    sprintf(tmpl, "/tmp/%s-XXXXXX%s", defaultPrefix, defaultSuffix);

    int fd = mkstemps(tmpl, (int) strlen(defaultSuffix));
    if (fd < 0)
    {
        free(tmpl);
        return NULL;
    }
    close(fd);
    return tmpl;
}

// Java: static long size(Path path)
long long LIBMATTI_JNF_Files_Size(const char *path)
{
    struct stat status;
    if (stat(path, &status) != 0)
        return -1;
    return (long long) status.st_size;
}

// Java: static boolean notExists(Path path, LinkOption... options)
int LIBMATTI_JNF_Files_NotExists(const char *path)
{
    return !LIBMATTI_JNF_Files_Exists(path);
}

// Java: static Path copy(Path source, Path target, CopyOption... options)
int LIBMATTI_JNF_Files_Copy(const char *source, const char *target)
{
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_JNF_Files_ReadAllBytes(source, &length);
    if (bytes == NULL)
        return 0;
    int written = LIBMATTI_JNF_Files_Write(target, bytes, length);
    free(bytes);
    return written;
}

// Java: static Path move(Path source, Path target, CopyOption... options)
int LIBMATTI_JNF_Files_Move(const char *source, const char *target)
{
    return rename(source, target) == 0;
}

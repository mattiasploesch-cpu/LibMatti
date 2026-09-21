// Port of java.util.zip.ZipFile.
// Java opens the central directory lazily on first access; the port reads it
// eagerly on open. ZIP64 is out of scope (Java supports it; the packs this port
// reads stay under the classic limits).

#ifndef MATTICRAFT_JAVA_UTIL_ZIP_ZIPFILE_H
#define MATTICRAFT_JAVA_UTIL_ZIP_ZIPFILE_H

#include <stddef.h>
#include <stdint.h>

// Java: java.util.zip.ZipEntry - the central directory record
typedef struct LIBMATTI_JU_ZipEntry
{
    char *name;
    int isDirectory;
    int method; // Java: STORED (0) / DEFLATED (8)
    uint32_t crc;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    long long localHeaderOffset;
} LIBMATTI_JU_ZipEntry;

// Java: java.util.zip.ZipFile
typedef struct LIBMATTI_JU_ZipFile LIBMATTI_JU_ZipFile;

// Java: public ZipFile(File file) throws ZipException - NULL on failure
LIBMATTI_JU_ZipFile *LIBMATTI_JU_ZipFile_Open(const char *path);
// Java: public void close()
void LIBMATTI_JU_ZipFile_Free(LIBMATTI_JU_ZipFile *zipFile);

// Java: public ZipEntry getEntry(String name) - NULL when the entry does not exist
const LIBMATTI_JU_ZipEntry *LIBMATTI_JU_ZipFile_GetEntry(const LIBMATTI_JU_ZipFile *zipFile, const char *name);
// Java: public Enumeration<? extends ZipEntry> entries() - the port exposes the array
size_t LIBMATTI_JU_ZipFile_EntryCount(const LIBMATTI_JU_ZipFile *zipFile);
const LIBMATTI_JU_ZipEntry *LIBMATTI_JU_ZipFile_EntryAt(const LIBMATTI_JU_ZipFile *zipFile, size_t index);

// Java: public InputStream getInputStream(ZipEntry entry) - returns the fully read
// (and for DEFLATED entries inflated) contents; NULL on failure, *outLength set.
// The caller frees the buffer.
unsigned char *LIBMATTI_JU_ZipFile_Read(const LIBMATTI_JU_ZipFile *zipFile, const LIBMATTI_JU_ZipEntry *entry,
                                        size_t *outLength);

#endif //MATTICRAFT_JAVA_UTIL_ZIP_ZIPFILE_H

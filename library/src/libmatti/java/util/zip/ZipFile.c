// Port of java.util.zip.ZipFile.
//
// Java parses the central directory on first access and reads entry data through
// the local file header on demand. The port keeps the file open for the ZipFile's
// lifetime, reads the central directory eagerly and inflates entry data on read.

#include "libmatti/java/util/zip/ZipFile.h"

#include "libmatti/java/util/zip/Zip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The signatures of the classic ZIP format
#define SIG_LOCAL_HEADER 0x04034b50u
#define SIG_CENTRAL_DIRECTORY 0x02014b50u
#define SIG_END_OF_CENTRAL_DIRECTORY 0x06054b50u

struct LIBMATTI_JU_ZipFile
{
    FILE *file;
    long long fileSize;
    // Java: the central directory records, in file order
    LIBMATTI_JU_ZipEntry **entries;
    size_t entryCount;
};

static uint16_t rd16(const unsigned char *p)
{
    return (uint16_t) ((uint16_t) p[0] | ((uint16_t) p[1] << 8));
}

static uint32_t rd32(const unsigned char *p)
{
    return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
}

LIBMATTI_JU_ZipFile *LIBMATTI_JU_ZipFile_Open(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return NULL;
    }
    long long size = ftell(file);

    // Java: the EOCD record is searched from the end; the comment may be up to
    // 64KiB, so the scan window covers it
    unsigned char maxScan[22 + 65536];
    long long scanSize = size < (long long) sizeof(maxScan) ? size : (long long) sizeof(maxScan);
    if (fseek(file, (long) (size - scanSize), SEEK_SET) != 0)
    {
        fclose(file);
        return NULL;
    }
    if (fread(maxScan, 1, (size_t) scanSize, file) != (size_t) scanSize)
    {
        fclose(file);
        return NULL;
    }

    // Java: ZipFile.open uses ZipFile.Source.findEND - the last EOCD in the window
    long long eocdOffset = -1;
    for (long long i = scanSize - 22; i >= 0; i--)
    {
        if (rd32(maxScan + i) == SIG_END_OF_CENTRAL_DIRECTORY)
        {
            eocdOffset = size - scanSize + i;
            break;
        }
    }
    if (eocdOffset < 0)
    {
        fclose(file);
        return NULL;
    }

    const unsigned char *eocd = maxScan + (eocdOffset - (size - scanSize));
    uint16_t entryCount = rd16(eocd + 10);
    uint32_t centralDirectoryOffset = rd32(eocd + 16);

    LIBMATTI_JU_ZipFile *zipFile = calloc(1, sizeof(LIBMATTI_JU_ZipFile));
    zipFile->file = file;
    zipFile->fileSize = size;
    zipFile->entryCount = entryCount;
    if (entryCount > 0)
        zipFile->entries = calloc(entryCount, sizeof(LIBMATTI_JU_ZipEntry *));

    // Java: Source.init - the central directory records follow each other back to back
    if (fseek(file, (long) centralDirectoryOffset, SEEK_SET) != 0)
    {
        LIBMATTI_JU_ZipFile_Free(zipFile);
        return NULL;
    }

    for (uint16_t i = 0; i < entryCount; i++)
    {
        unsigned char header[46];
        if (fread(header, 1, sizeof(header), file) != sizeof(header) || rd32(header) != SIG_CENTRAL_DIRECTORY)
        {
            LIBMATTI_JU_ZipFile_Free(zipFile);
            return NULL;
        }

        LIBMATTI_JU_ZipEntry *entry = calloc(1, sizeof(LIBMATTI_JU_ZipEntry));
        entry->method = rd16(header + 10);
        entry->crc = rd32(header + 16);
        entry->compressedSize = rd32(header + 20);
        entry->uncompressedSize = rd32(header + 24);
        entry->localHeaderOffset = (long long) rd32(header + 42);
        uint16_t nameLength = rd16(header + 28);
        uint16_t extraLength = rd16(header + 30);
        uint16_t commentLength = rd16(header + 32);

        entry->name = malloc((size_t) nameLength + 1);
        if (nameLength > 0 && fread(entry->name, 1, nameLength, file) != nameLength)
        {
            LIBMATTI_JU_ZipFile_Free(zipFile);
            return NULL;
        }
        entry->name[nameLength] = '\0';
        entry->isDirectory = nameLength > 0 && entry->name[nameLength - 1] == '/';
        if (entry->isDirectory) entry->name[nameLength - 1] = '\0';

        // Java: skips the extra field and the comment to the next record
        size_t skip = (size_t) extraLength + commentLength;
        unsigned char *junk = malloc(skip);
        if (skip > 0 && fread(junk, 1, skip, file) != skip)
        {
            free(junk);
            LIBMATTI_JU_ZipFile_Free(zipFile);
            return NULL;
        }
        free(junk);

        zipFile->entries[i] = entry;
    }

    return zipFile;
}

void LIBMATTI_JU_ZipFile_Free(LIBMATTI_JU_ZipFile *zipFile)
{
    if (zipFile == NULL) return;
    if (zipFile->file != NULL) fclose(zipFile->file);
    for (size_t i = 0; i < zipFile->entryCount; i++)
    {
        free(zipFile->entries[i]->name);
        free(zipFile->entries[i]);
    }
    free(zipFile->entries);
    free(zipFile);
}

const LIBMATTI_JU_ZipEntry *LIBMATTI_JU_ZipFile_GetEntry(const LIBMATTI_JU_ZipFile *zipFile, const char *name)
{
    for (size_t i = 0; i < zipFile->entryCount; i++)
    {
        if (strcmp(zipFile->entries[i]->name, name) == 0) return zipFile->entries[i];
    }
    return NULL;
}

size_t LIBMATTI_JU_ZipFile_EntryCount(const LIBMATTI_JU_ZipFile *zipFile)
{
    return zipFile->entryCount;
}

const LIBMATTI_JU_ZipEntry *LIBMATTI_JU_ZipFile_EntryAt(const LIBMATTI_JU_ZipFile *zipFile, size_t index)
{
    if (index >= zipFile->entryCount) return NULL;
    return zipFile->entries[index];
}

unsigned char *LIBMATTI_JU_ZipFile_Read(const LIBMATTI_JU_ZipFile *zipFile, const LIBMATTI_JU_ZipEntry *entry,
                                        size_t *outLength)
{
    *outLength = 0;
    // Java: directories read to an empty stream
    if (entry->isDirectory || entry->uncompressedSize == 0)
        return calloc(1, 1);

    // Java: getInputStream reads through the local file header - the lengths there can be
    // zero with a data descriptor, so the sizes come from the central directory
    unsigned char header[30];
    if (fseek(zipFile->file, (long) entry->localHeaderOffset, SEEK_SET) != 0) return NULL;
    if (fread(header, 1, sizeof(header), zipFile->file) != sizeof(header)) return NULL;
    if (rd32(header) != SIG_LOCAL_HEADER) return NULL;
    uint16_t nameLength = rd16(header + 26);
    uint16_t extraLength = rd16(header + 28);
    long long dataOffset = entry->localHeaderOffset + 30 + nameLength + extraLength;

    unsigned char *compressed = malloc(entry->compressedSize);
    if (fseek(zipFile->file, (long) dataOffset, SEEK_SET) != 0)
    {
        free(compressed);
        return NULL;
    }
    if (entry->compressedSize > 0 && fread(compressed, 1, entry->compressedSize, zipFile->file) != entry->compressedSize)
    {
        free(compressed);
        return NULL;
    }

    // Java: ZipEntry.STORED entries are copied; DEFLATED entries run through the inflater
    unsigned char *data;
    if (entry->method == 0)
    {
        data = compressed;
        *outLength = entry->uncompressedSize;
    }
    else
    {
        data = LIBMATTI_JU_Inflater_Decompress(compressed, entry->compressedSize, entry->uncompressedSize, outLength);
        free(compressed);
        if (data == NULL) return NULL;
    }
    return data;
}

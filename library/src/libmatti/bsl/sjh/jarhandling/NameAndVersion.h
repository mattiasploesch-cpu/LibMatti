





#ifndef MATTICRAFT_NAMEANDVERSION_H
#define MATTICRAFT_NAMEANDVERSION_H

// Java: record NameAndVersion(String name, String version)
typedef struct
{
    char *name;
    char *version;

} LIBMATTI_JH_NameAndVersion;

LIBMATTI_JH_NameAndVersion *LIBMATTI_JH_NameAndVersion_Create(const char *name, const char *version);
void LIBMATTI_JH_NameAndVersion_Free(LIBMATTI_JH_NameAndVersion *nameAndVersion);

#endif

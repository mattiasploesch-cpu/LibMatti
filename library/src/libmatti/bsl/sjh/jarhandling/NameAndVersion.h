//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.NameAndVersion.

#ifndef MATTICRAFT_NAMEANDVERSION_H
#define MATTICRAFT_NAMEANDVERSION_H

// Java: record NameAndVersion(String name, String version)
typedef struct
{
    char *name;
    char *version; // NULL = no version
} LIBMATTI_JH_NameAndVersion;

LIBMATTI_JH_NameAndVersion *LIBMATTI_JH_NameAndVersion_Create(const char *name, const char *version);
void LIBMATTI_JH_NameAndVersion_Free(LIBMATTI_JH_NameAndVersion *nameAndVersion);

#endif //MATTICRAFT_NAMEANDVERSION_H
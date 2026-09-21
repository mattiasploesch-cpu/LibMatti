// Port of net.neoforged.fml.jarmoduleinfo.NameAndVersion.

#ifndef MATTICRAFT_FML_JARMODULEINFO_NAMEANDVERSION_H
#define MATTICRAFT_FML_JARMODULEINFO_NAMEANDVERSION_H

// Java: record NameAndVersion(String name, @Nullable String version) {}
typedef struct
{
    char *name;
    // Java: @Nullable String version
    char *version;
} LIBMATTI_FML_NameAndVersion;

void LIBMATTI_FML_NameAndVersion_Free(LIBMATTI_FML_NameAndVersion *nameAndVersion);

#endif //MATTICRAFT_FML_JARMODULEINFO_NAMEANDVERSION_H

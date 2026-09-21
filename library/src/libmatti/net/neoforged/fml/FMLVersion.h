// Port of net.neoforged.fml.FMLVersion.

#ifndef MATTICRAFT_FML_FMLVERSION_H
#define MATTICRAFT_FML_FMLVERSION_H

// Java: private FMLVersion() {}
typedef struct LIBMATTI_FML_FMLVersion
{
    int unused;
} LIBMATTI_FML_FMLVersion;

// Java: public static String getVersion()
const char *LIBMATTI_FML_FMLVersion_GetVersion(void);

#endif //MATTICRAFT_FML_FMLVERSION_H

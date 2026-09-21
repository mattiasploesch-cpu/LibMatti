// Port of net.minecraft.server.packs.PackType.

#ifndef MATTICRAFT_MC_PACKS_PACKTYPE_H
#define MATTICRAFT_MC_PACKS_PACKTYPE_H

// Java: public enum PackType
typedef enum
{
    LIBMATTI_MC_PackType_CLIENT_RESOURCES = 0,
    LIBMATTI_MC_PackType_SERVER_DATA = 1
} LIBMATTI_MC_PackType;

// Java: public String getDirectory()
const char *LIBMATTI_MC_PackType_GetDirectory(LIBMATTI_MC_PackType type);

#endif //MATTICRAFT_MC_PACKS_PACKTYPE_H

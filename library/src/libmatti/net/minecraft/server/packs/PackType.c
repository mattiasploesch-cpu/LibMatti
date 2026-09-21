// Port of net.minecraft.server.packs.PackType.

#include "libmatti/net/minecraft/server/packs/PackType.h"

const char *LIBMATTI_MC_PackType_GetDirectory(LIBMATTI_MC_PackType type)
{
    switch (type)
    {
        case LIBMATTI_MC_PackType_CLIENT_RESOURCES: return "assets";
        case LIBMATTI_MC_PackType_SERVER_DATA: return "data";
    }
    return "assets";
}

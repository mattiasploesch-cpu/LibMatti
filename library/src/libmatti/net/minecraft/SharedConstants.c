// Port of net.minecraft.SharedConstants.

#include "libmatti/net/minecraft/SharedConstants.h"

int LIBMATTI_MC_SharedConstants_IS_RUNNING_IN_IDE = 0;

int LIBMATTI_MC_SharedConstants_GetResourcePackVersionMajor(void)
{
    return LIBMATTI_MC_SharedConstants_RESOURCE_PACK_FORMAT_MAJOR;
}

int LIBMATTI_MC_SharedConstants_GetResourcePackVersionMinor(void)
{
    return LIBMATTI_MC_SharedConstants_RESOURCE_PACK_FORMAT_MINOR;
}

int LIBMATTI_MC_SharedConstants_GetDataPackVersionMajor(void)
{
    return LIBMATTI_MC_SharedConstants_DATA_PACK_FORMAT_MAJOR;
}

int LIBMATTI_MC_SharedConstants_GetDataPackVersionMinor(void)
{
    return LIBMATTI_MC_SharedConstants_DATA_PACK_FORMAT_MINOR;
}

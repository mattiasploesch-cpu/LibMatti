// Port of net.neoforged.fml.common.asm.enumextension.NetworkedEnum.

#include "libmatti/net/neoforged/fml/common/asm/enumextension/NetworkedEnum.h"

#include <stddef.h>

// Java: enum NetworkCheck.name()
const char *LIBMATTI_FML_NetworkedEnum_NetworkCheck_Name(LIBMATTI_FML_NetworkedEnum_NetworkCheck check)
{
    switch (check)
    {
        case LIBMATTI_FML_NETWORKEDENUM_CLIENTBOUND: return "CLIENTBOUND";
        case LIBMATTI_FML_NETWORKEDENUM_SERVERBOUND: return "SERVERBOUND";
        case LIBMATTI_FML_NETWORKEDENUM_BIDIRECTIONAL: return "BIDIRECTIONAL";
    }

    return NULL;
}

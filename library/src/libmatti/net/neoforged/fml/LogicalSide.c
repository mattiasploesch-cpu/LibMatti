// Port of net.neoforged.fml.LogicalSide.

#include "libmatti/net/neoforged/fml/LogicalSide.h"

int LIBMATTI_FML_LogicalSide_IsClient(LIBMATTI_FML_LogicalSide side)
{
    return side == LIBMATTI_FML_LogicalSide_CLIENT;
}

// Java: return !isClient();
int LIBMATTI_FML_LogicalSide_IsServer(LIBMATTI_FML_LogicalSide side)
{
    return !LIBMATTI_FML_LogicalSide_IsClient(side);
}

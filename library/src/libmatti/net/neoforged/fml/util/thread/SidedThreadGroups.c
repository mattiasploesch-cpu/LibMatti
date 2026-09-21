// Port of net.neoforged.fml.util.thread.SidedThreadGroups.

#include "libmatti/net/neoforged/fml/util/thread/SidedThreadGroups.h"

// Java: public static final SidedThreadGroup CLIENT = new SidedThreadGroup(LogicalSide.CLIENT);
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroups_CLIENT(void)
{
    static LIBMATTI_FML_SidedThreadGroup *client = NULL;
    if (client == NULL) client = LIBMATTI_FML_SidedThreadGroup_New(LIBMATTI_FML_LogicalSide_CLIENT);
    return client;
}

// Java: public static final SidedThreadGroup SERVER = new SidedThreadGroup(LogicalSide.SERVER);
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroups_SERVER(void)
{
    static LIBMATTI_FML_SidedThreadGroup *server = NULL;
    if (server == NULL) server = LIBMATTI_FML_SidedThreadGroup_New(LIBMATTI_FML_LogicalSide_SERVER);
    return server;
}

// Port of net.neoforged.fml.util.thread.EffectiveSide.

#include "libmatti/net/neoforged/fml/util/thread/EffectiveSide.h"

#include "libmatti/java/lang/Thread.h"
#include "libmatti/net/neoforged/fml/util/thread/SidedThreadGroup.h"

// Java: ThreadGroup group = Thread.currentThread().getThreadGroup();
//       return group instanceof SidedThreadGroup ? ((SidedThreadGroup) group).getSide() : LogicalSide.CLIENT;
LIBMATTI_FML_LogicalSide LIBMATTI_FML_EffectiveSide_Get(void)
{
    LIBMATTI_FML_SidedThreadGroup *group =
        LIBMATTI_FML_SidedThreadGroup_GetThreadGroup(LIBMATTI_JL_Thread_CurrentThread());
    return group != NULL ? LIBMATTI_FML_SidedThreadGroup_GetSide(group) : LIBMATTI_FML_LogicalSide_CLIENT;
}

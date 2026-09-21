// Port of net.neoforged.fml.util.thread.SidedThreadGroups.

#ifndef MATTICRAFT_FML_UTIL_THREAD_SIDEDTHREADGROUPS_H
#define MATTICRAFT_FML_UTIL_THREAD_SIDEDTHREADGROUPS_H

#include "libmatti/net/neoforged/fml/util/thread/SidedThreadGroup.h"

// Java: public static final SidedThreadGroup CLIENT
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroups_CLIENT(void);
// Java: public static final SidedThreadGroup SERVER
LIBMATTI_FML_SidedThreadGroup *LIBMATTI_FML_SidedThreadGroups_SERVER(void);

#endif //MATTICRAFT_FML_UTIL_THREAD_SIDEDTHREADGROUPS_H

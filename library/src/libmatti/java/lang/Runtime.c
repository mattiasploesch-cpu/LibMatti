#include "libmatti/java/lang/Runtime.h"

#include <unistd.h>

// Java: public static Runtime getRuntime()
LIBMATTI_JL_Runtime *LIBMATTI_JL_Runtime_GetRuntime(void)
{
    static LIBMATTI_JL_Runtime runtime = {0};
    return &runtime;
}

// Java: public int availableProcessors()
int LIBMATTI_JL_Runtime_AvailableProcessors(void)
{
    long processors = sysconf(_SC_NPROCESSORS_ONLN);
    return processors > 0 ? (int) processors : 1;
}

// Java: public long maxMemory()
long LIBMATTI_JL_Runtime_MaxMemory(void)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long pageSize = sysconf(_SC_PAGESIZE);
    return pages > 0 && pageSize > 0 ? pages * pageSize : 0;
}

// Java: public long totalMemory()
long LIBMATTI_JL_Runtime_TotalMemory(void)
{
    return LIBMATTI_JL_Runtime_MaxMemory();
}

// Java: public long freeMemory()
long LIBMATTI_JL_Runtime_FreeMemory(void)
{
    return LIBMATTI_JL_Runtime_MaxMemory();
}

// Java: public static int version()
int LIBMATTI_JL_Runtime_Version(void)
{
    return 0;
}

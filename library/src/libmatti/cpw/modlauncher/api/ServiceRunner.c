// Port of cpw.mods.modlauncher.api.ServiceRunner.

#include "libmatti/cpw/modlauncher/api/ServiceRunner.h"

void *LIBMATTI_MLA_ServiceRunner_Run(LIBMATTI_MLA_ServiceRunner *runner, void *classLoader)
{
    return runner->run(runner, classLoader);
}

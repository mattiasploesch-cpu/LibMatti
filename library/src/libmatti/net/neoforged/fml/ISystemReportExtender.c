// Port of net.neoforged.fml.ISystemReportExtender.

#include "libmatti/net/neoforged/fml/ISystemReportExtender.h"

#include <stddef.h>

char *LIBMATTI_FML_ISystemReportExtender_GetLabel(LIBMATTI_FML_ISystemReportExtender *extender)
{
    return extender->getLabel(extender);
}

int LIBMATTI_FML_ISystemReportExtender_IsActive(LIBMATTI_FML_ISystemReportExtender *extender)
{
    // Java: default boolean isActive() { return true; }
    if (extender->isActive == NULL) return 1;
    return extender->isActive(extender);
}

char *LIBMATTI_FML_ISystemReportExtender_Get(LIBMATTI_FML_ISystemReportExtender *extender)
{
    return extender->get(extender);
}

// Port of net.neoforged.fml.ISystemReportExtender.

#ifndef MATTICRAFT_FML_ISYSTEMREPORTEXTENDER_H
#define MATTICRAFT_FML_ISYSTEMREPORTEXTENDER_H

// Java: @ApiStatus.OverrideOnly public interface ISystemReportExtender extends Supplier<String>
typedef struct LIBMATTI_FML_ISystemReportExtender
{
    // Java: String getLabel()
    char *(*getLabel)(struct LIBMATTI_FML_ISystemReportExtender *self);
    // Java: default boolean isActive() { return true; }
    int (*isActive)(struct LIBMATTI_FML_ISystemReportExtender *self);
    // Java: String get() (from Supplier<String>)
    char *(*get)(struct LIBMATTI_FML_ISystemReportExtender *self);
} LIBMATTI_FML_ISystemReportExtender;

// Java: the interface methods, dispatched to the implementing vtable
char *LIBMATTI_FML_ISystemReportExtender_GetLabel(LIBMATTI_FML_ISystemReportExtender *extender);
int LIBMATTI_FML_ISystemReportExtender_IsActive(LIBMATTI_FML_ISystemReportExtender *extender);
char *LIBMATTI_FML_ISystemReportExtender_Get(LIBMATTI_FML_ISystemReportExtender *extender);

#endif //MATTICRAFT_FML_ISYSTEMREPORTEXTENDER_H

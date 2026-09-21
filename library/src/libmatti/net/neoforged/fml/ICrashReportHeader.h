// Port of net.neoforged.fml.ICrashReportHeader.

#ifndef MATTICRAFT_FML_ICRASHREPORTHEADER_H
#define MATTICRAFT_FML_ICRASHREPORTHEADER_H

// Java: public interface ICrashReportHeader
typedef struct LIBMATTI_FML_ICrashReportHeader
{
    // Java: String getHeader()
    char *(*getHeader)(struct LIBMATTI_FML_ICrashReportHeader *self);
} LIBMATTI_FML_ICrashReportHeader;

// Java: the interface method, dispatched to the implementing vtable
char *LIBMATTI_FML_ICrashReportHeader_GetHeader(LIBMATTI_FML_ICrashReportHeader *header);

#endif //MATTICRAFT_FML_ICRASHREPORTHEADER_H

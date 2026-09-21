// Port of net.neoforged.fml.ICrashReportHeader.

#include "libmatti/net/neoforged/fml/ICrashReportHeader.h"

char *LIBMATTI_FML_ICrashReportHeader_GetHeader(LIBMATTI_FML_ICrashReportHeader *header)
{
    return header->getHeader(header);
}

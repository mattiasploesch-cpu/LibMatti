// Port of com.electronwill.nightconfig.core.io.ParsingMode.

#ifndef MATTICRAFT_NC_CORE_IO_PARSINGMODE_H
#define MATTICRAFT_NC_CORE_IO_PARSINGMODE_H

#include "libmatti/com/electronwill/nightconfig/core/Config.h"

// Java: public enum ParsingMode { REPLACE, MERGE, ADD }
typedef enum
{
    LIBMATTI_NC_PARSINGMODE_REPLACE = 0,
    LIBMATTI_NC_PARSINGMODE_MERGE,
    LIBMATTI_NC_PARSINGMODE_ADD
} LIBMATTI_NC_ParsingMode;

// Java: public void prepareParsing(Config config)
void LIBMATTI_NC_ParsingMode_PrepareParsing(LIBMATTI_NC_ParsingMode mode, LIBMATTI_NC_Config *config);
// Java: public Object put(Config config, List<String> key, Object value)
LIBMATTI_NC_Value *LIBMATTI_NC_ParsingMode_Put(LIBMATTI_NC_ParsingMode mode, LIBMATTI_NC_Config *config,
                                               const char **path, size_t pathLength,
                                               const LIBMATTI_NC_Value *value);

#endif //MATTICRAFT_NC_CORE_IO_PARSINGMODE_H

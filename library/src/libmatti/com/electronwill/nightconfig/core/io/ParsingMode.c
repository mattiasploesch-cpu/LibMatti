// Port of com.electronwill.nightconfig.core.io.ParsingMode.

#include "libmatti/com/electronwill/nightconfig/core/io/ParsingMode.h"

void LIBMATTI_NC_ParsingMode_PrepareParsing(LIBMATTI_NC_ParsingMode mode, LIBMATTI_NC_Config *config)
{
    // Java: REPLACE clears the config, MERGE and ADD keep it
    if (mode == LIBMATTI_NC_PARSINGMODE_REPLACE) LIBMATTI_NC_Config_Clear(config);
}

LIBMATTI_NC_Value *LIBMATTI_NC_ParsingMode_Put(LIBMATTI_NC_ParsingMode mode, LIBMATTI_NC_Config *config,
                                               const char **path, size_t pathLength,
                                               const LIBMATTI_NC_Value *value)
{
    // Java: MERGE sets the value, ADD keeps an existing value
    if (mode == LIBMATTI_NC_PARSINGMODE_ADD) return LIBMATTI_NC_Config_Add(config, path, pathLength, value);
    return LIBMATTI_NC_Config_Set(config, path, pathLength, value);
}

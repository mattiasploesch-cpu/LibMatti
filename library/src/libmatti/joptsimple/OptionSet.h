// Port of joptsimple.OptionSet.

#ifndef MATTICRAFT_JOPTSIMPLE_OPTIONSET_H
#define MATTICRAFT_JOPTSIMPLE_OPTIONSET_H

#include "libmatti/joptsimple/OptionParser.h"

#include <stddef.h>

// Java: public class OptionSet
typedef struct LIBMATTI_JOPT_OptionSet
{
    LIBMATTI_JOPT_OptionParser *parser;
    char **nonOptions;
    size_t nonOptionCount;
} LIBMATTI_JOPT_OptionSet;

// Java: boolean has(OptionSpec<?> option)
int LIBMATTI_JOPT_OptionSet_Has(const LIBMATTI_JOPT_OptionSet *set, const LIBMATTI_JOPT_OptionSpec *spec);
// Java: V valueOf(OptionSpec<V> option)
const char *LIBMATTI_JOPT_OptionSet_ValueOf(const LIBMATTI_JOPT_OptionSet *set, const LIBMATTI_JOPT_OptionSpec *spec);
// Java: List<V> valuesOf(OptionSpec<V> option)
char **LIBMATTI_JOPT_OptionSet_ValuesOf(const LIBMATTI_JOPT_OptionSet *set, const LIBMATTI_JOPT_OptionSpec *spec,
                                        size_t *count);
// Java: List<?> nonOptionArguments()
char **LIBMATTI_JOPT_OptionSet_NonOptionArguments(const LIBMATTI_JOPT_OptionSet *set, size_t *count);

void LIBMATTI_JOPT_OptionSet_Free(LIBMATTI_JOPT_OptionSet *set);

#endif //MATTICRAFT_JOPTSIMPLE_OPTIONSET_H

// Port of joptsimple.OptionSpec.

#include "libmatti/joptsimple/OptionSpec.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionSpec_WithRequiredArg(LIBMATTI_JOPT_OptionSpec *spec)
{
    spec->hasArg = 1;
    return spec;
}

LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionSpec_DefaultsTo(LIBMATTI_JOPT_OptionSpec *spec, const char *value)
{
    free(spec->defaultValue);
    spec->defaultValue = strdup(value);
    return spec;
}

LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionSpec_WithValuesConvertedBy(LIBMATTI_JOPT_OptionSpec *spec,
                                                                        void *converter)
{
    spec->converter = converter;
    return spec;
}

const char *LIBMATTI_JOPT_OptionSpec_Options(const LIBMATTI_JOPT_OptionSpec *spec)
{
    return spec->name;
}

const char *LIBMATTI_JOPT_OptionSpec_Value(const LIBMATTI_JOPT_OptionSpec *spec)
{
    // Java: AbstractOptionSpec.value(OptionSet) returns the default when absent
    if (spec->valueCount > 0) return spec->values[spec->valueCount - 1];
    return spec->defaultValue;
}

void LIBMATTI_JOPT_OptionSpec_Free(LIBMATTI_JOPT_OptionSpec *spec)
{
    if (spec == NULL) return;
    free(spec->name);
    free(spec->description);
    free(spec->defaultValue);
    for (size_t i = 0; i < spec->valueCount; i++) free(spec->values[i]);
    free(spec->values);
    free(spec);
}

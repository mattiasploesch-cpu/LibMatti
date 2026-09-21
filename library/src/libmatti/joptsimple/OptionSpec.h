// Port of joptsimple.OptionSpec / OptionSpecBuilder / AbstractOptionSpec.
// joptsimple is an external dependency; the C port models the option as a name,
// an optional required argument and the parsed values. Java's OptionSpec<V>
// generic is expressed as strings.

#ifndef MATTICRAFT_JOPTSIMPLE_OPTIONSPEC_H
#define MATTICRAFT_JOPTSIMPLE_OPTIONSPEC_H

#include <stddef.h>

typedef struct LIBMATTI_JOPT_OptionSpec
{
    char *name;
    char *description;
    // Java: withRequiredArg()
    int hasArg;
    // Java: defaultsTo(...)
    char *defaultValue;
    // Java: withValuesConvertedBy(...) - the converter is not applied in the C port
    void *converter;
    // parse state
    int present;
    char **values;
    size_t valueCount;
} LIBMATTI_JOPT_OptionSpec;

// Java: OptionSpecBuilder.withRequiredArg()
LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionSpec_WithRequiredArg(LIBMATTI_JOPT_OptionSpec *spec);
// Java: AbstractOptionSpec.defaultsTo(V defaultValue)
LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionSpec_DefaultsTo(LIBMATTI_JOPT_OptionSpec *spec, const char *value);
// Java: ArgumentAcceptingOptionSpec.withValuesConvertedBy(ArgumentConverter converter)
LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionSpec_WithValuesConvertedBy(LIBMATTI_JOPT_OptionSpec *spec,
                                                                        void *converter);
// Java: List<String> options()
const char *LIBMATTI_JOPT_OptionSpec_Options(const LIBMATTI_JOPT_OptionSpec *spec);
// Java: V value(OptionSet set)
const char *LIBMATTI_JOPT_OptionSpec_Value(const LIBMATTI_JOPT_OptionSpec *spec);

void LIBMATTI_JOPT_OptionSpec_Free(LIBMATTI_JOPT_OptionSpec *spec);

#endif //MATTICRAFT_JOPTSIMPLE_OPTIONSPEC_H

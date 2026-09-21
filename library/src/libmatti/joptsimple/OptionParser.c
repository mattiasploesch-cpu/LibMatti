// Port of joptsimple.OptionParser and joptsimple.OptionSet.

#include "libmatti/joptsimple/OptionParser.h"
#include "libmatti/joptsimple/OptionSet.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JOPT_OptionParser *LIBMATTI_JOPT_OptionParser_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JOPT_OptionParser));
}

void LIBMATTI_JOPT_OptionParser_Free(LIBMATTI_JOPT_OptionParser *parser)
{
    if (parser == NULL) return;

    for (size_t i = 0; i < parser->count; i++) LIBMATTI_JOPT_OptionSpec_Free(parser->specs[i]);
    free(parser->specs);
    free(parser);
}

void LIBMATTI_JOPT_OptionParser_AllowsUnrecognizedOptions(LIBMATTI_JOPT_OptionParser *parser)
{
    parser->allowsUnrecognized = 1;
}

LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionParser_Accepts(LIBMATTI_JOPT_OptionParser *parser, const char *option,
                                                            const char *description)
{
    LIBMATTI_JOPT_OptionSpec *spec = calloc(1, sizeof(LIBMATTI_JOPT_OptionSpec));
    spec->name = strdup(option);
    spec->description = description != NULL ? strdup(description) : NULL;

    parser->specs = realloc(parser->specs, sizeof(*parser->specs) * (parser->count + 1));
    parser->specs[parser->count++] = spec;

    return spec;
}

LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionParser_AcceptsFlag(LIBMATTI_JOPT_OptionParser *parser, const char *option)
{
    return LIBMATTI_JOPT_OptionParser_Accepts(parser, option, NULL);
}

LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionParser_NonOptions(LIBMATTI_JOPT_OptionParser *parser)
{
    return LIBMATTI_JOPT_OptionParser_Accepts(parser, "", NULL);
}

LIBMATTI_JOPT_OptionSet *LIBMATTI_JOPT_OptionParser_Parse(LIBMATTI_JOPT_OptionParser *parser, int argc, char *argv[])
{
    LIBMATTI_JOPT_OptionSet *set = calloc(1, sizeof(LIBMATTI_JOPT_OptionSet));
    set->parser = parser;

    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            const char *name = argv[i] + 2;

            LIBMATTI_JOPT_OptionSpec *spec = NULL;
            for (size_t s = 0; s < parser->count; s++)
                if (strcmp(parser->specs[s]->name, name) == 0)
                {
                    spec = parser->specs[s];
                    break;
                }

            if (spec != NULL && spec->name[0] != '\0')
            {
                spec->present = 1;
                if (spec->hasArg && i + 1 < argc)
                {
                    spec->values = realloc(spec->values, sizeof(*spec->values) * (spec->valueCount + 1));
                    spec->values[spec->valueCount++] = strdup(argv[++i]);
                }
            }
            else if (parser->allowsUnrecognized)
            {
                // Java: allowsUnrecognizedOptions() keeps the unrecognized option as a non-option argument,
                // so it is handed back through nonOptionArguments() (this is how --fml.* reach FMLLoader).
                set->nonOptions = realloc(set->nonOptions, sizeof(*set->nonOptions) * (set->nonOptionCount + 1));
                set->nonOptions[set->nonOptionCount++] = argv[i];
            }
            continue;
        }

        set->nonOptions = realloc(set->nonOptions, sizeof(*set->nonOptions) * (set->nonOptionCount + 1));
        set->nonOptions[set->nonOptionCount++] = argv[i];
    }

    return set;
}

int LIBMATTI_JOPT_OptionSet_Has(const LIBMATTI_JOPT_OptionSet *set, const LIBMATTI_JOPT_OptionSpec *spec)
{
    (void)set;
    return spec->present;
}

const char *LIBMATTI_JOPT_OptionSet_ValueOf(const LIBMATTI_JOPT_OptionSet *set, const LIBMATTI_JOPT_OptionSpec *spec)
{
    (void)set;
    return LIBMATTI_JOPT_OptionSpec_Value(spec);
}

char **LIBMATTI_JOPT_OptionSet_ValuesOf(const LIBMATTI_JOPT_OptionSet *set, const LIBMATTI_JOPT_OptionSpec *spec,
                                        size_t *count)
{
    (void)set;
    *count = spec->valueCount;
    return spec->values;
}

char **LIBMATTI_JOPT_OptionSet_NonOptionArguments(const LIBMATTI_JOPT_OptionSet *set, size_t *count)
{
    *count = set->nonOptionCount;
    return set->nonOptions;
}

void LIBMATTI_JOPT_OptionSet_Free(LIBMATTI_JOPT_OptionSet *set)
{
    if (set == NULL) return;
    free(set->nonOptions);
    free(set);
}

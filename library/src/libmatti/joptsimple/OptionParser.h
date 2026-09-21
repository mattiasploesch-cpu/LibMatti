// Port of joptsimple.OptionParser.

#ifndef MATTICRAFT_JOPTSIMPLE_OPTIONPARSER_H
#define MATTICRAFT_JOPTSIMPLE_OPTIONPARSER_H

#include "libmatti/joptsimple/OptionSpec.h"

#include <stddef.h>

typedef struct LIBMATTI_JOPT_OptionSet LIBMATTI_JOPT_OptionSet;

// Java: public class OptionParser
typedef struct LIBMATTI_JOPT_OptionParser
{
    LIBMATTI_JOPT_OptionSpec **specs;
    size_t count;
    int allowsUnrecognized;
} LIBMATTI_JOPT_OptionParser;

// Java: public OptionParser()
LIBMATTI_JOPT_OptionParser *LIBMATTI_JOPT_OptionParser_New(void);
void LIBMATTI_JOPT_OptionParser_Free(LIBMATTI_JOPT_OptionParser *parser);

// Java: public void allowsUnrecognizedOptions()
void LIBMATTI_JOPT_OptionParser_AllowsUnrecognizedOptions(LIBMATTI_JOPT_OptionParser *parser);
// Java: public OptionSpecBuilder accepts(String option, String description)
LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionParser_Accepts(LIBMATTI_JOPT_OptionParser *parser, const char *option,
                                                            const char *description);
// Java: public OptionSpec<Boolean> accepts(String option)
LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionParser_AcceptsFlag(LIBMATTI_JOPT_OptionParser *parser, const char *option);
// Java: public NonOptionArgumentSpec<String> nonOptions()
LIBMATTI_JOPT_OptionSpec *LIBMATTI_JOPT_OptionParser_NonOptions(LIBMATTI_JOPT_OptionParser *parser);
// Java: public OptionSet parse(String... arguments)
LIBMATTI_JOPT_OptionSet *LIBMATTI_JOPT_OptionParser_Parse(LIBMATTI_JOPT_OptionParser *parser, int argc, char *argv[]);

#endif //MATTICRAFT_JOPTSIMPLE_OPTIONPARSER_H

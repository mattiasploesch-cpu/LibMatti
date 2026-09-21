// Port of com.electronwill.nightconfig.toml.ArrayParser.

#include "libmatti/com/electronwill/nightconfig/toml/ArrayParser.h"

#include "libmatti/com/electronwill/nightconfig/toml/Toml.h"
#include "libmatti/com/electronwill/nightconfig/toml/ValueParser.h"

// Java: static List<?> parse(CharacterInput input, TomlParser parser, CommentedConfig parentConfig)
LIBMATTI_NC_ValueResult LIBMATTI_NC_ArrayParser_Parse(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser,
                                                      LIBMATTI_NC_Config *parentConfig)
{
    LIBMATTI_NC_Value list = LIBMATTI_NC_Value_OfList(NULL, 0);
    int first = 1;
    while (1)
    {
        int firstChar = LIBMATTI_NC_Toml_ReadUsefulChar(input);
        if (firstChar == -1)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid array, the closing bracket is missing");
            goto fail;
        }
        if (firstChar == ']') // End of the array
        {
            return LIBMATTI_NC_ValueResult_Ok(list);
        }
        if (firstChar == ',') // Handles [,] or [v1,,] which are both invalid
        {
            LIBMATTI_NC_TomlParser_Error(
                parser, first ? "Invalid array: [,]" : "Invalid double comma in array.");
            goto fail;
        }

        LIBMATTI_NC_ValueResult value = LIBMATTI_NC_ValueParser_Parse(input, (char) firstChar, parser, parentConfig);
        if (!value.ok) goto fail;
        LIBMATTI_NC_Value_ListAppend(&list, value.value);

        int after = LIBMATTI_NC_Toml_ReadUsefulChar(input);
        if (after == ']') // End of the array
        {
            return LIBMATTI_NC_ValueResult_Ok(list);
        }
        if (after != ',') // Invalid character between two elements of the array
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid separator in array");
            goto fail;
        }
        first = 0;
    }

fail:
    LIBMATTI_NC_Value_Free(&list);
    return LIBMATTI_NC_ValueResult_Fail();
}

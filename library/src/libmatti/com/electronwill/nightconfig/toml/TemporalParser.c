// Port of com.electronwill.nightconfig.toml.TemporalParser.

#include "libmatti/com/electronwill/nightconfig/toml/TemporalParser.h"

#include "libmatti/com/electronwill/nightconfig/core/io/Utils.h"

#include <string.h>

// Java: private static final char[] ALLOWED_DT_SEPARATORS = {'T', 't', ' '}
static const char ALLOWED_DT_SEPARATORS[] = {'T', 't', ' '};
// Java: private static final char[] OFFSET_INDICATORS = {'Z', 'z', '+', '-'}
static const char OFFSET_INDICATORS[] = {'Z', 'z', '+', '-'};

static int contains_char(const char *chars, size_t length, char value)
{
    for (size_t i = 0; i < length; i++)
        if (chars[i] == value) return 1;
    return 0;
}

// Java: private static LocalDate parseDate(CharsWrapper chars)
static int parse_date(const LIBMATTI_NC_CharsWrapper *chars, LIBMATTI_NC_TomlParser *parser,
                      LIBMATTI_JT_Temporal *out)
{
    LIBMATTI_NC_CharsWrapper yearChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 0, 4);
    LIBMATTI_NC_CharsWrapper monthChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 5, 7);
    LIBMATTI_NC_CharsWrapper dayChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 8, 10);

    int ok = 0;
    int year = LIBMATTI_NC_Utils_ParseInt(&yearChars, 10, &ok);
    if (!ok)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid temporal value: invalid year");
        return 0;
    }
    int month = LIBMATTI_NC_Utils_ParseInt(&monthChars, 10, &ok);
    if (!ok)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid temporal value: invalid month");
        return 0;
    }
    int day = LIBMATTI_NC_Utils_ParseInt(&dayChars, 10, &ok);
    if (!ok)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid temporal value: invalid day");
        return 0;
    }

    const char *error = NULL;
    if (!LIBMATTI_JT_LocalDate_Of(year, month, day, out, &error))
    {
        LIBMATTI_NC_TomlParser_Error(parser, error);
        return 0;
    }
    return 1;
}

// Java: private static LocalTime parseTime(CharsWrapper chars, TomlVersion version)
static int parse_time(const LIBMATTI_NC_CharsWrapper *chars, LIBMATTI_NC_TomlVersion version,
                      LIBMATTI_NC_TomlParser *parser, LIBMATTI_JT_Temporal *out)
{
    // hours and minutes are always there
    LIBMATTI_NC_CharsWrapper hourChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 0, 2);
    LIBMATTI_NC_CharsWrapper minuteChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 3, 5);

    int ok = 0;
    int hour = LIBMATTI_NC_Utils_ParseInt(&hourChars, 10, &ok);
    if (!ok)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid temporal value: invalid hour");
        return 0;
    }
    int minutes = LIBMATTI_NC_Utils_ParseInt(&minuteChars, 10, &ok);
    if (!ok)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid temporal value: invalid minutes");
        return 0;
    }
    int seconds = 0;

    // seconds can be omitted since TOML v1.1
    if (chars->length > 5)
    {
        LIBMATTI_NC_CharsWrapper secondChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 6, 8);
        seconds = LIBMATTI_NC_Utils_ParseInt(&secondChars, 10, &ok);
        if (!ok)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid temporal value: invalid seconds");
            return 0;
        }
    }
    else if (version == LIBMATTI_NC_TOMLVERSION_V1_0)
    {
        LIBMATTI_NC_TomlParser_Error(parser,
                                     "Invalid time: missing seconds. NOTE: this is rejected in TOML v1.0, but accepted in TOML v1.1");
        return 0;
    }

    // nanoseconds are optional
    int nanos = 0;
    if (chars->length > 8)
    {
        LIBMATTI_NC_CharsWrapper fractionChars = LIBMATTI_NC_CharsWrapper_SubView(chars, 9);
        if (fractionChars.length > 9)
            fractionChars.length = 9; // Java: truncates if too many digits

        int value = LIBMATTI_NC_Utils_ParseInt(&fractionChars, 10, &ok);
        if (!ok)
        {
            LIBMATTI_NC_TomlParser_Error(parser, "Invalid temporal value: invalid fraction of second");
            return 0;
        }
        int coefficient = 1;
        for (size_t i = fractionChars.length; i < 9; i++)
            coefficient *= 10;
        nanos = value * coefficient;
    }

    *out = LIBMATTI_JT_LocalTime_Of(hour, minutes, seconds, nanos);
    return 1;
}

// Java: private static ZoneOffset parseOffset(CharsWrapper chars)
static int parse_offset(const LIBMATTI_NC_CharsWrapper *chars, LIBMATTI_NC_TomlParser *parser, int *outSeconds)
{
    // TOML is more strict than Java's ZoneOffset.
    // For instance, java accepts +0209, but TOML only accepts +02:09
    long colon = LIBMATTI_NC_CharsWrapper_IndexOfFirst(chars, ":", 1);
    if (colon == -1 && !LIBMATTI_NC_CharsWrapper_ContentEquals(chars, "Z", 1))
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid time offset");
        return 0;
    }

    // Java: ZoneOffset.of(chars.toString()) - the id is Z or +HH:MM
    if (LIBMATTI_NC_CharsWrapper_ContentEquals(chars, "Z", 1))
    {
        *outSeconds = 0;
        return 1;
    }

    LIBMATTI_NC_CharsWrapper sign = LIBMATTI_NC_CharsWrapper_SubView2(chars, 0, 1);
    LIBMATTI_NC_CharsWrapper hourChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 1, 3);
    LIBMATTI_NC_CharsWrapper minuteChars = LIBMATTI_NC_CharsWrapper_SubView2(chars, 4, 6);

    int negative = sign.data[0] == '-';

    int ok = 0;
    int hours = LIBMATTI_NC_Utils_ParseInt(&hourChars, 10, &ok);
    if (!ok || hours > 18)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid time offset");
        return 0;
    }
    int minutes = LIBMATTI_NC_Utils_ParseInt(&minuteChars, 10, &ok);
    if (!ok || minutes > 59)
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid time offset");
        return 0;
    }

    int seconds = hours * 3600 + minutes * 60;
    *outSeconds = negative ? -seconds : seconds;
    return 1;
}

// Java: static Temporal parse(CharsWrapper chars, TomlVersion version)
int LIBMATTI_NC_TemporalParser_Parse(const LIBMATTI_NC_CharsWrapper *chars, LIBMATTI_NC_TomlVersion version,
                                     LIBMATTI_NC_TomlParser *parser, LIBMATTI_JT_Temporal *out)
{
    // Java: chars = chars.trimmedView() - trim the trailing spaces (a comment may follow)
    LIBMATTI_NC_CharsWrapper value = LIBMATTI_NC_CharsWrapper_TrimmedView(chars);

    if (value.length > 2 && value.data[2] == ':')
    { // LocalTime
        return parse_time(&value, version, parser, out);
    }

    LIBMATTI_JT_Temporal date;
    if (!parse_date(&value, parser, &date)) return 0;
    if (value.length == 10)
    { // LocalDate
        *out = date;
        return 1;
    }

    char dateTimeSeparator = value.data[10];
    if (!contains_char(ALLOWED_DT_SEPARATORS, sizeof(ALLOWED_DT_SEPARATORS), dateTimeSeparator))
    {
        LIBMATTI_NC_TomlParser_Error(parser, "Invalid separator between date and time");
        return 0;
    }

    LIBMATTI_NC_CharsWrapper afterDate = LIBMATTI_NC_CharsWrapper_SubView(&value, 11);
    long offsetIndicatorIndex = LIBMATTI_NC_CharsWrapper_IndexOfFirst(&afterDate, OFFSET_INDICATORS,
                                                                     sizeof(OFFSET_INDICATORS));
    if (offsetIndicatorIndex == -1)
    { // LocalDateTime
        LIBMATTI_JT_Temporal time;
        if (!parse_time(&afterDate, version, parser, &time)) return 0;
        *out = LIBMATTI_JT_LocalDateTime_Of(&date, &time);
        return 1;
    }

    if (afterDate.data[offsetIndicatorIndex] == 'z')
        afterDate.data[offsetIndicatorIndex] = 'Z'; // Java does not support 'z' but only 'Z'

    LIBMATTI_NC_CharsWrapper timeChars = LIBMATTI_NC_CharsWrapper_SubView2(&afterDate, 0, (size_t) offsetIndicatorIndex);
    LIBMATTI_JT_Temporal time;
    if (!parse_time(&timeChars, version, parser, &time)) return 0;

    LIBMATTI_NC_CharsWrapper offsetView = {afterDate.data + offsetIndicatorIndex,
                                           afterDate.length - (size_t) offsetIndicatorIndex};
    LIBMATTI_NC_CharsWrapper offsetChars = LIBMATTI_NC_CharsWrapper_TrimmedView(&offsetView);
    int offsetSeconds = 0;
    if (!parse_offset(&offsetChars, parser, &offsetSeconds)) return 0;

    *out = LIBMATTI_JT_OffsetDateTime_Of(&date, &time, offsetSeconds); // OffsetDateTime
    return 1;
}

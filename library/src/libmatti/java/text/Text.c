#include "libmatti/java/text/Text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *LIBMATTI_JT_MessageFormat_Format(const char *pattern, const char *const *values, size_t count)
{
    // Java: the '{' opens a placeholder, digits index the argument, '}' closes;
    // '\'' quotes the next character
    size_t capacity = strlen(pattern) + 64;
    char *out = malloc(capacity);
    size_t outIndex = 0;

    for (size_t i = 0; pattern[i] != '\0'; i++)
    {
        char ch = pattern[i];

        if (ch == '\'' && pattern[i + 1] != '\0')
        {
            // Java: two quotes are a literal quote, one quotes the next char
            out[outIndex++] = pattern[++i];
            continue;
        }

        if (ch == '{')
        {
            int index = 0, hasDigits = 0;
            while (pattern[i + 1] >= '0' && pattern[i + 1] <= '9')
            {
                index = index * 10 + (pattern[++i] - '0');
                hasDigits = 1;
            }
            if (pattern[i + 1] == '}')
                i++;
            if (hasDigits && (size_t) index < count)
            {
                const char *value = values[index] != NULL ? values[index] : "null";
                size_t valueLength = strlen(value);
                if (outIndex + valueLength + 1 > capacity)
                {
                    capacity = (outIndex + valueLength + 1) * 2;
                    out = realloc(out, capacity);
                }
                memcpy(out + outIndex, value, valueLength);
                outIndex += valueLength;
            }
            continue;
        }

        out[outIndex++] = ch;
    }
    out[outIndex] = '\0';
    return out;
}

char *LIBMATTI_JT_DateFormat_FormatEpoch(long long epochMillis, const char *pattern)
{
    // Java: SimpleDateFormat in UTC (the log format uses UTC)
    time_t seconds = (time_t) (epochMillis / 1000);
    struct tm tm_utc;
    gmtime_r(&seconds, &tm_utc);
    int millis = (int) (epochMillis % 1000);

    char buffer[64];
    char *cursor = buffer;

    for (size_t i = 0; pattern[i] != '\0'; i++)
    {
        char token = pattern[i];
        int run = 1;
        while (pattern[i + 1] == token)
        {
            i++;
            run++;
        }

        switch (token)
        {
            case 'y':
                cursor += sprintf(cursor, run >= 4 ? "%04d" : "%02d", tm_utc.tm_year + 1900);
                break;
            case 'M':
                cursor += sprintf(cursor, "%02d", tm_utc.tm_mon + 1);
                break;
            case 'd':
                cursor += sprintf(cursor, "%02d", tm_utc.tm_mday);
                break;
            case 'H':
                cursor += sprintf(cursor, "%02d", tm_utc.tm_hour);
                break;
            case 'm':
                cursor += sprintf(cursor, "%02d", tm_utc.tm_min);
                break;
            case 's':
                cursor += sprintf(cursor, "%02d", tm_utc.tm_sec);
                break;
            case 'S':
                cursor += sprintf(cursor, "%03d", millis);
                break;
            default:
                *cursor++ = token;
                for (int r = 1; r < run; r++)
                    *cursor++ = token;
                break;
        }
    }
    *cursor = '\0';
    return strdup(buffer);
}

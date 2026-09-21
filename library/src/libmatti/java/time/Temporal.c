// Port of the java.time types the TOML layer needs (see Temporal.h).

#include "libmatti/java/time/Temporal.h"

// Java: LocalDate.of validates the date and throws DateTimeException
int LIBMATTI_JT_LocalDate_Of(int year, int month, int dayOfMonth, LIBMATTI_JT_Temporal *out, const char **error)
{
    static const int LENGTHS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12)
    {
        *error = "Invalid value for MonthOfYear";
        return 0;
    }

    int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    int length = month == 2 && leap ? 29 : LENGTHS[month - 1];
    if (dayOfMonth < 1 || dayOfMonth > length)
    {
        *error = "Invalid value for DayOfMonth";
        return 0;
    }

    out->kind = LIBMATTI_JT_TEMPORAL_LOCAL_DATE;
    out->year = year;
    out->month = month;
    out->dayOfMonth = dayOfMonth;
    out->hour = 0;
    out->minute = 0;
    out->second = 0;
    out->nano = 0;
    out->offsetSeconds = 0;
    return 1;
}

// Java: static LocalTime of(int hour, int minute, int second, int nanoOfSecond)
LIBMATTI_JT_Temporal LIBMATTI_JT_LocalTime_Of(int hour, int minute, int second, int nanoOfSecond)
{
    LIBMATTI_JT_Temporal temporal = {0};
    temporal.kind = LIBMATTI_JT_TEMPORAL_LOCAL_TIME;
    temporal.hour = hour;
    temporal.minute = minute;
    temporal.second = second;
    temporal.nano = nanoOfSecond;
    return temporal;
}

// Java: static LocalDateTime of(LocalDate date, LocalTime time)
LIBMATTI_JT_Temporal LIBMATTI_JT_LocalDateTime_Of(const LIBMATTI_JT_Temporal *date, const LIBMATTI_JT_Temporal *time)
{
    LIBMATTI_JT_Temporal temporal = *date;
    temporal.kind = LIBMATTI_JT_TEMPORAL_LOCAL_DATE_TIME;
    temporal.hour = time->hour;
    temporal.minute = time->minute;
    temporal.second = time->second;
    temporal.nano = time->nano;
    temporal.offsetSeconds = 0;
    return temporal;
}

// Java: static OffsetDateTime of(LocalDate date, LocalTime time, ZoneOffset offset)
LIBMATTI_JT_Temporal LIBMATTI_JT_OffsetDateTime_Of(const LIBMATTI_JT_Temporal *date, const LIBMATTI_JT_Temporal *time,
                                                  int offsetSeconds)
{
    LIBMATTI_JT_Temporal temporal = LIBMATTI_JT_LocalDateTime_Of(date, time);
    temporal.kind = LIBMATTI_JT_TEMPORAL_OFFSET_DATE_TIME;
    temporal.offsetSeconds = offsetSeconds;
    return temporal;
}

// Java: isSupported(ChronoField.YEAR)
int LIBMATTI_JT_Temporal_SupportsYear(const LIBMATTI_JT_Temporal *temporal)
{
    return temporal->kind == LIBMATTI_JT_TEMPORAL_LOCAL_DATE ||
           temporal->kind == LIBMATTI_JT_TEMPORAL_LOCAL_DATE_TIME ||
           temporal->kind == LIBMATTI_JT_TEMPORAL_OFFSET_DATE_TIME;
}

// Java: isSupported(ChronoField.HOUR_OF_DAY)
int LIBMATTI_JT_Temporal_SupportsHour(const LIBMATTI_JT_Temporal *temporal)
{
    return temporal->kind == LIBMATTI_JT_TEMPORAL_LOCAL_TIME ||
           temporal->kind == LIBMATTI_JT_TEMPORAL_LOCAL_DATE_TIME ||
           temporal->kind == LIBMATTI_JT_TEMPORAL_OFFSET_DATE_TIME;
}

// Java: isSupported(ChronoField.OFFSET_SECONDS)
int LIBMATTI_JT_Temporal_SupportsOffsetSeconds(const LIBMATTI_JT_Temporal *temporal)
{
    return temporal->kind == LIBMATTI_JT_TEMPORAL_OFFSET_DATE_TIME;
}

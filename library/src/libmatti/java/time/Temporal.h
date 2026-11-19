





#ifndef MATTICRAFT_JAVA_TIME_TEMPORAL_H
#define MATTICRAFT_JAVA_TIME_TEMPORAL_H

// Java: the concrete Temporal implementations
typedef enum
{
    LIBMATTI_JT_TEMPORAL_LOCAL_DATE = 0,
    LIBMATTI_JT_TEMPORAL_LOCAL_TIME,
    LIBMATTI_JT_TEMPORAL_LOCAL_DATE_TIME,
    LIBMATTI_JT_TEMPORAL_OFFSET_DATE_TIME
} LIBMATTI_JT_TemporalKind;

typedef struct LIBMATTI_JT_Temporal
{
    LIBMATTI_JT_TemporalKind kind;



    int year;
    int month;// Java: MONTH_OF_YEAR, 1-12
    int dayOfMonth;// Java: DAY_OF_MONTH



    int hour;// Java: HOUR_OF_DAY
    int minute;// Java: MINUTE_OF_HOUR
    int second;// Java: SECOND_OF_MINUTE
    int nano;// Java: NANO_OF_SECOND

// OffsetDateTime only: Java: ZoneOffset, kept as total seconds
    int offsetSeconds;
} LIBMATTI_JT_Temporal;

// Java: static LocalTime of(int hour, int minute, int second, int nanoOfSecond)
LIBMATTI_JT_Temporal LIBMATTI_JT_LocalTime_Of(int hour, int minute, int second, int nanoOfSecond);
// Java: static LocalDate of(int year, int month, int dayOfMonth)
// DateTimeException for an invalid date, like Java (LocalDate.of validates).
// Returns 0 and writes the exception message when the date is invalid.
int LIBMATTI_JT_LocalDate_Of(int year, int month, int dayOfMonth, LIBMATTI_JT_Temporal *out, const char **error);
// Java: static LocalDateTime of(LocalDate date, LocalTime time)
LIBMATTI_JT_Temporal LIBMATTI_JT_LocalDateTime_Of(const LIBMATTI_JT_Temporal *date, const LIBMATTI_JT_Temporal *time);
// Java: static OffsetDateTime of(LocalDate date, LocalTime time, ZoneOffset offset)
LIBMATTI_JT_Temporal LIBMATTI_JT_OffsetDateTime_Of(const LIBMATTI_JT_Temporal *date, const LIBMATTI_JT_Temporal *time,
                                                  int offsetSeconds);

// Java: boolean isSupported(TemporalField) for the fields TemporalWriter uses
int LIBMATTI_JT_Temporal_SupportsYear(const LIBMATTI_JT_Temporal *temporal);
int LIBMATTI_JT_Temporal_SupportsHour(const LIBMATTI_JT_Temporal *temporal);
int LIBMATTI_JT_Temporal_SupportsOffsetSeconds(const LIBMATTI_JT_Temporal *temporal);

#endif

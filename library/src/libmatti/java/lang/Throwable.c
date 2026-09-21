#include "Throwable.h"

#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/java/lang/System.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_JL_Throwable *LIBMATTI_JL_Throwable_New(const char *message)
{
    return LIBMATTI_JL_Throwable_NewNamed("java.lang.Throwable", message);
}

LIBMATTI_JL_Throwable *LIBMATTI_JL_Throwable_NewNamed(const char *className, const char *message)
{
    LIBMATTI_JL_Throwable *throwable = calloc(1, sizeof(LIBMATTI_JL_Throwable));
    throwable->className = className != NULL ? strdup(className) : NULL;
    throwable->message = message != NULL ? strdup(message) : NULL;
    return throwable;
}

static void throwable_free_inner(LIBMATTI_JL_Throwable *throwable, int freeSelf)
{
    if (throwable == NULL) return;

    free(throwable->className);
    free(throwable->message);

    for (size_t i = 0; i < throwable->stackTraceCount; i++)
    {
        free(throwable->stackTrace[i].className);
        free(throwable->stackTrace[i].methodName);
        free(throwable->stackTrace[i].fileName);
    }
    free(throwable->stackTrace);

    for (size_t i = 0; i < throwable->suppressedCount; i++)
        throwable_free_inner(throwable->suppressed[i], 1);
    free(throwable->suppressed);

    throwable_free_inner(throwable->cause, 1);

    if (freeSelf) free(throwable);
}

void LIBMATTI_JL_Throwable_Free(LIBMATTI_JL_Throwable *throwable)
{
    throwable_free_inner(throwable, 1);
}

const char *LIBMATTI_JL_Throwable_GetName(const LIBMATTI_JL_Throwable *throwable)
{
    return throwable->className;
}

const char *LIBMATTI_JL_Throwable_GetMessage(const LIBMATTI_JL_Throwable *throwable)
{
    return throwable->message;
}

LIBMATTI_JL_Throwable *LIBMATTI_JL_Throwable_GetCause(const LIBMATTI_JL_Throwable *throwable)
{
    return throwable->cause;
}

void LIBMATTI_JL_Throwable_SetCause(LIBMATTI_JL_Throwable *throwable, LIBMATTI_JL_Throwable *cause)
{
    throwable->cause = cause;
}

LIBMATTI_JL_Throwable **LIBMATTI_JL_Throwable_GetSuppressed(const LIBMATTI_JL_Throwable *throwable, size_t *count)
{
    *count = throwable->suppressedCount;
    return throwable->suppressed;
}

void LIBMATTI_JL_Throwable_AddSuppressed(LIBMATTI_JL_Throwable *throwable, LIBMATTI_JL_Throwable *suppressed)
{
    throwable->suppressed = realloc(throwable->suppressed,
                                    sizeof(*throwable->suppressed) * (throwable->suppressedCount + 1));
    throwable->suppressed[throwable->suppressedCount++] = suppressed;
}

const LIBMATTI_JL_StackTraceElement *LIBMATTI_JL_Throwable_GetStackTrace(const LIBMATTI_JL_Throwable *throwable,
                                                                         size_t *count)
{
    *count = throwable->stackTraceCount;
    return throwable->stackTrace;
}

void LIBMATTI_JL_Throwable_SetStackTrace(LIBMATTI_JL_Throwable *throwable,
                                         const LIBMATTI_JL_StackTraceElement *stackTrace, size_t count)
{
    throwable->stackTrace = calloc(count, sizeof(*throwable->stackTrace));

    for (size_t i = 0; i < count; i++)
    {
        throwable->stackTrace[i].className = stackTrace[i].className != NULL ? strdup(stackTrace[i].className) : NULL;
        throwable->stackTrace[i].methodName =
            stackTrace[i].methodName != NULL ? strdup(stackTrace[i].methodName) : NULL;
        throwable->stackTrace[i].fileName = stackTrace[i].fileName != NULL ? strdup(stackTrace[i].fileName) : NULL;
        throwable->stackTrace[i].lineNumber = stackTrace[i].lineNumber;
        throwable->stackTrace[i].nativeMethod = stackTrace[i].nativeMethod;
    }

    throwable->stackTraceCount = count;
}

// Java: the set of already printed throwables (identity), used to break cycles
typedef struct
{
    const LIBMATTI_JL_Throwable **items;
    size_t count;
    size_t capacity;
} DejaVu;

static void deja_vu_add(DejaVu *dejaVu, const LIBMATTI_JL_Throwable *throwable)
{
    if (dejaVu->count == dejaVu->capacity)
    {
        dejaVu->capacity = dejaVu->capacity == 0 ? 8 : dejaVu->capacity * 2;
        dejaVu->items = realloc(dejaVu->items, sizeof(*dejaVu->items) * dejaVu->capacity);
    }
    dejaVu->items[dejaVu->count++] = throwable;
}

static int deja_vu_contains(const DejaVu *dejaVu, const LIBMATTI_JL_Throwable *throwable)
{
    for (size_t i = 0; i < dejaVu->count; i++)
        if (dejaVu->items[i] == throwable) return 1;
    return 0;
}

// Java: Throwable.toString()
static char *throwable_to_string(const LIBMATTI_JL_Throwable *throwable)
{
    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_New();
    LIBMATTI_JL_StringBuilder_Append(builder, throwable->className);
    if (throwable->message != NULL)
    {
        LIBMATTI_JL_StringBuilder_Append(builder, ": ");
        LIBMATTI_JL_StringBuilder_Append(builder, throwable->message);
    }

    char *result = strdup(LIBMATTI_JL_StringBuilder_ToString(builder));
    LIBMATTI_JL_StringBuilder_Free(builder);
    return result;
}

// Java: StackTraceElement.toString()
static char *stack_trace_element_to_string(const LIBMATTI_JL_StackTraceElement *element)
{
    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_New();
    LIBMATTI_JL_StringBuilder_Append(builder, element->className);
    LIBMATTI_JL_StringBuilder_AppendChar(builder, '.');
    LIBMATTI_JL_StringBuilder_Append(builder, element->methodName);

    if (element->nativeMethod)
    {
        LIBMATTI_JL_StringBuilder_Append(builder, "(Native Method)");
    }
    else if (element->fileName != NULL && element->lineNumber >= 0)
    {
        char location[512];
        snprintf(location, sizeof(location), "(%s:%d)", element->fileName, element->lineNumber);
        LIBMATTI_JL_StringBuilder_Append(builder, location);
    }
    else if (element->fileName != NULL)
    {
        char location[512];
        snprintf(location, sizeof(location), "(%s)", element->fileName);
        LIBMATTI_JL_StringBuilder_Append(builder, location);
    }
    else
    {
        LIBMATTI_JL_StringBuilder_Append(builder, "(Unknown Source)");
    }

    char *result = strdup(LIBMATTI_JL_StringBuilder_ToString(builder));
    LIBMATTI_JL_StringBuilder_Free(builder);
    return result;
}

// Java: StackTraceElement.equals(Object)
static int stack_trace_element_equals(const LIBMATTI_JL_StackTraceElement *a, const LIBMATTI_JL_StackTraceElement *b)
{
    return strcmp(a->className, b->className) == 0
           && strcmp(a->methodName, b->methodName) == 0
           && a->fileName != NULL && b->fileName != NULL && strcmp(a->fileName, b->fileName) == 0
           && a->lineNumber == b->lineNumber
           && a->nativeMethod == b->nativeMethod;
}

// Java: s.println(a + b + c + d)
static void println_parts(LIBMATTI_JI_PrintWriter *writer, const char *a, const char *b, const char *c, const char *d)
{
    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_New();
    LIBMATTI_JL_StringBuilder_Append(builder, a);
    LIBMATTI_JL_StringBuilder_Append(builder, b);
    LIBMATTI_JL_StringBuilder_Append(builder, c);
    LIBMATTI_JL_StringBuilder_Append(builder, d);
    LIBMATTI_JI_PrintWriter_Println(writer, LIBMATTI_JL_StringBuilder_ToString(builder));
    LIBMATTI_JL_StringBuilder_Free(builder);
}

// Java: private void printEnclosedStackTrace(PrintWriter s, StackTraceElement[] enclosingTrace,
//                                            String caption, String prefix, Set<Throwable> dejaVu)
static void print_enclosed_stack_trace(const LIBMATTI_JL_Throwable *throwable,
                                       const LIBMATTI_JL_StackTraceElement *enclosingTrace, size_t enclosingCount,
                                       const char *caption, const char *prefix, DejaVu *dejaVu,
                                       LIBMATTI_JI_PrintWriter *writer)
{
    if (deja_vu_contains(dejaVu, throwable))
    {
        // Java: s.println(prefix + caption + "[CIRCULAR REFERENCE: " + this + "]")
        char *text = throwable_to_string(throwable);
        println_parts(writer, prefix, caption, "[CIRCULAR REFERENCE: ", text);
        LIBMATTI_JI_PrintWriter_Println(writer, "]");
        free(text);
        return;
    }
    deja_vu_add(dejaVu, throwable);

    size_t m = throwable->stackTraceCount;
    size_t n = enclosingCount;
    while (m > 0 && n > 0
           && stack_trace_element_equals(&throwable->stackTrace[m - 1], &enclosingTrace[n - 1]))
    {
        m--;
        n--;
    }
    size_t framesInCommon = throwable->stackTraceCount - m;

    char *text = throwable_to_string(throwable);
    println_parts(writer, prefix, caption, text, "");
    free(text);

    for (size_t i = 0; i < m; i++)
    {
        char *element = stack_trace_element_to_string(&throwable->stackTrace[i]);
        println_parts(writer, prefix, "\tat ", element, "");
        free(element);
    }

    if (framesInCommon != 0)
    {
        char count[32];
        snprintf(count, sizeof(count), "%zu", framesInCommon);
        println_parts(writer, prefix, "\t... ", count, " more");
    }

    for (size_t i = 0; i < throwable->suppressedCount; i++)
    {
        LIBMATTI_JL_StringBuilder *nestedPrefix = LIBMATTI_JL_StringBuilder_NewFromString(prefix);
        LIBMATTI_JL_StringBuilder_Append(nestedPrefix, "\t");
        print_enclosed_stack_trace(throwable->suppressed[i], throwable->stackTrace, throwable->stackTraceCount,
                                   "Suppressed: ", LIBMATTI_JL_StringBuilder_ToString(nestedPrefix), dejaVu, writer);
        LIBMATTI_JL_StringBuilder_Free(nestedPrefix);
    }

    if (throwable->cause != NULL)
    {
        print_enclosed_stack_trace(throwable->cause, throwable->stackTrace, throwable->stackTraceCount,
                                   "Caused by: ", prefix, dejaVu, writer);
    }
}

// Java: public void printStackTrace(PrintWriter s)
void LIBMATTI_JL_Throwable_PrintStackTraceTo(const LIBMATTI_JL_Throwable *throwable, LIBMATTI_JI_PrintWriter *writer)
{
    DejaVu dejaVu = {NULL, 0, 0};
    deja_vu_add(&dejaVu, throwable);

    char *text = throwable_to_string(throwable);
    LIBMATTI_JI_PrintWriter_Println(writer, text);
    free(text);

    for (size_t i = 0; i < throwable->stackTraceCount; i++)
    {
        char *element = stack_trace_element_to_string(&throwable->stackTrace[i]);
        println_parts(writer, "\tat ", element, "", "");
        free(element);
    }

    for (size_t i = 0; i < throwable->suppressedCount; i++)
    {
        print_enclosed_stack_trace(throwable->suppressed[i], throwable->stackTrace, throwable->stackTraceCount,
                                   "Suppressed: ", "\t", &dejaVu, writer);
    }

    if (throwable->cause != NULL)
    {
        print_enclosed_stack_trace(throwable->cause, throwable->stackTrace, throwable->stackTraceCount,
                                   "Caused by: ", "", &dejaVu, writer);
    }

    free(dejaVu.items);
}

// Java: public void printStackTrace() { printStackTrace(System.err); }
void LIBMATTI_JL_Throwable_PrintStackTrace(const LIBMATTI_JL_Throwable *throwable)
{
    LIBMATTI_JL_Throwable_PrintStackTraceTo(throwable, LIBMATTI_JL_System_Err());
}

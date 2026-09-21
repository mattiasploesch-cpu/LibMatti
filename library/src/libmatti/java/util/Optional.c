#include "libmatti/java/util/Optional.h"

#include <stdio.h>
#include <stdlib.h>

LIBMATTI_JU_Optional LIBMATTI_JU_Optional_Empty(void)
{
    LIBMATTI_JU_Optional optional = {0};
    return optional;
}

LIBMATTI_JU_Optional LIBMATTI_JU_Optional_Of(void *value)
{
    if (value == NULL)
    {
        // Java: throw new NullPointerException("No value present")
        fprintf(stderr, "Exception in thread \"main\" java.lang.NullPointerException: No value present\n");
        abort();
    }
    LIBMATTI_JU_Optional optional = {value};
    return optional;
}

LIBMATTI_JU_Optional LIBMATTI_JU_Optional_OfNullable(void *value)
{
    LIBMATTI_JU_Optional optional = {value};
    return optional;
}

void *LIBMATTI_JU_Optional_Get(const LIBMATTI_JU_Optional *optional)
{
    if (optional->value == NULL)
    {
        // Java: throw new NoSuchElementException("No value present")
        fprintf(stderr, "Exception in thread \"main\" java.util.NoSuchElementException: No value present\n");
        abort();
    }
    return optional->value;
}

int LIBMATTI_JU_Optional_IsPresent(const LIBMATTI_JU_Optional *optional)
{
    return optional->value != NULL;
}

int LIBMATTI_JU_Optional_IsEmpty(const LIBMATTI_JU_Optional *optional)
{
    return optional->value == NULL;
}

void *LIBMATTI_JU_Optional_OrElse(const LIBMATTI_JU_Optional *optional, void *other)
{
    return optional->value != NULL ? optional->value : other;
}

void *LIBMATTI_JU_Optional_OrElseGet(const LIBMATTI_JU_Optional *optional, void *(*supplier)(void *self), void *self)
{
    return optional->value != NULL ? optional->value : supplier(self);
}

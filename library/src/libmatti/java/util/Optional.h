// Port of java.util.Optional<T>.
// In the C port T is void* and "empty" means the value pointer is NULL.

#ifndef MATTICRAFT_JAVA_UTIL_OPTIONAL_H
#define MATTICRAFT_JAVA_UTIL_OPTIONAL_H

#include <stddef.h>

typedef struct
{
    void *value; // NULL = empty
} LIBMATTI_JU_Optional;

// Java: public static <T> Optional<T> empty()
LIBMATTI_JU_Optional LIBMATTI_JU_Optional_Empty(void);
// Java: public static <T> Optional<T> of(T value) - crashes on NULL like Java's NPE
LIBMATTI_JU_Optional LIBMATTI_JU_Optional_Of(void *value);
// Java: public static <T> Optional<T> ofNullable(T value)
LIBMATTI_JU_Optional LIBMATTI_JU_Optional_OfNullable(void *value);

// Java: public T get()
void *LIBMATTI_JU_Optional_Get(const LIBMATTI_JU_Optional *optional);
// Java: public boolean isPresent()
int LIBMATTI_JU_Optional_IsPresent(const LIBMATTI_JU_Optional *optional);
// Java: public boolean isEmpty()
int LIBMATTI_JU_Optional_IsEmpty(const LIBMATTI_JU_Optional *optional);
// Java: public T orElse(T other)
void *LIBMATTI_JU_Optional_OrElse(const LIBMATTI_JU_Optional *optional, void *other);
// Java: public T orElseGet(Supplier<? extends T> supplier)
void *LIBMATTI_JU_Optional_OrElseGet(const LIBMATTI_JU_Optional *optional, void *(*supplier)(void *self), void *self);

#endif //MATTICRAFT_JAVA_UTIL_OPTIONAL_H

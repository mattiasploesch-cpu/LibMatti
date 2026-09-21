// Port of java.util.function.Predicate.
// Java is generic; the C port erases the type argument to void*.

#ifndef MATTICRAFT_JAVA_UTIL_FUNCTION_PREDICATE_H
#define MATTICRAFT_JAVA_UTIL_FUNCTION_PREDICATE_H

// Java: public interface Predicate<T>
typedef struct LIBMATTI_JU_Predicate LIBMATTI_JU_Predicate;

struct LIBMATTI_JU_Predicate
{
    void *self;

    // Java: boolean test(T t);
    int (*test)(void *self, void *value);
};

// Java: boolean test(T t)
int LIBMATTI_JU_Predicate_Test(const LIBMATTI_JU_Predicate *predicate, void *value);

#endif //MATTICRAFT_JAVA_UTIL_FUNCTION_PREDICATE_H

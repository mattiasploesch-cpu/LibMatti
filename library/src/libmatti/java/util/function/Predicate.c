// Port of java.util.function.Predicate.

#include "libmatti/java/util/function/Predicate.h"

// Java: boolean test(T t)
int LIBMATTI_JU_Predicate_Test(const LIBMATTI_JU_Predicate *predicate, void *value)
{
    return predicate->test(predicate->self, value);
}

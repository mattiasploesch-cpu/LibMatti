// Port of java.util.function.Consumer.

#include "libmatti/java/util/function/Consumer.h"

// Java: void accept(T t)
void LIBMATTI_JU_Consumer_Accept(const LIBMATTI_JU_Consumer *consumer, void *value)
{
    consumer->accept(consumer->self, value);
}

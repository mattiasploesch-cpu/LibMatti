// Port of java.util.function.Consumer.
// Java is generic; the C port erases the type argument to void*.

#ifndef MATTICRAFT_JAVA_UTIL_FUNCTION_CONSUMER_H
#define MATTICRAFT_JAVA_UTIL_FUNCTION_CONSUMER_H

// Java: public interface Consumer<T>
typedef struct LIBMATTI_JU_Consumer LIBMATTI_JU_Consumer;

struct LIBMATTI_JU_Consumer
{
    void *self;

    // Java: void accept(T t);
    void (*accept)(void *self, void *value);
};

// Java: void accept(T t)
void LIBMATTI_JU_Consumer_Accept(const LIBMATTI_JU_Consumer *consumer, void *value);

#endif //MATTICRAFT_JAVA_UTIL_FUNCTION_CONSUMER_H

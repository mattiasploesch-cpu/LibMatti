// Port of java.util.function.Supplier.
// Java is generic; the C port erases the type argument to void*.

#ifndef MATTICRAFT_JAVA_UTIL_FUNCTION_SUPPLIER_H
#define MATTICRAFT_JAVA_UTIL_FUNCTION_SUPPLIER_H

// Java: public interface Supplier<T>
typedef struct LIBMATTI_JU_Supplier LIBMATTI_JU_Supplier;

struct LIBMATTI_JU_Supplier
{
    void *self;

    // Java: T get();
    void *(*get)(void *self);
};

// Java: T get()
void *LIBMATTI_JU_Supplier_Get(const LIBMATTI_JU_Supplier *supplier);

#endif //MATTICRAFT_JAVA_UTIL_FUNCTION_SUPPLIER_H

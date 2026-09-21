// Port of cpw.mods.modlauncher.api.LambdaExceptionUtils.
// Java wraps lambdas that throw so an unchecked throw can escape a stream
// pipeline. C has no checked exceptions and no lambdas: the callback is the
// exception-carrying interface, and the wrappers are the identity.

#ifndef MATTICRAFT_MODLAUNCHER_LAMBDAEXCEPTIONUTILS_H
#define MATTICRAFT_MODLAUNCHER_LAMBDAEXCEPTIONUTILS_H

// Java: interface Consumer_WithExceptions<T, E extends Exception> { void accept(T t) throws E; }
typedef struct LIBMATTI_MLA_Consumer_WithExceptions
{
    void (*accept)(struct LIBMATTI_MLA_Consumer_WithExceptions *self, void *value);
    void *userdata;
} LIBMATTI_MLA_Consumer_WithExceptions;

// Java: interface BiConsumer_WithExceptions<T, U, E extends Exception>
typedef struct LIBMATTI_MLA_BiConsumer_WithExceptions
{
    void (*accept)(struct LIBMATTI_MLA_BiConsumer_WithExceptions *self, void *first, void *second);
    void *userdata;
} LIBMATTI_MLA_BiConsumer_WithExceptions;

// Java: interface Function_WithExceptions<T, R, E extends Exception>
typedef struct LIBMATTI_MLA_Function_WithExceptions
{
    void *(*apply)(struct LIBMATTI_MLA_Function_WithExceptions *self, void *value);
    void *userdata;
} LIBMATTI_MLA_Function_WithExceptions;

// Java: interface Supplier_WithExceptions<T, E extends Exception>
typedef struct LIBMATTI_MLA_Supplier_WithExceptions
{
    void *(*get)(struct LIBMATTI_MLA_Supplier_WithExceptions *self);
    void *userdata;
} LIBMATTI_MLA_Supplier_WithExceptions;

// Java: interface Runnable_WithExceptions<E extends Exception>
typedef struct LIBMATTI_MLA_Runnable_WithExceptions
{
    void (*run)(struct LIBMATTI_MLA_Runnable_WithExceptions *self);
    void *userdata;
} LIBMATTI_MLA_Runnable_WithExceptions;

// Java: the unchecked target interfaces (java.util.function.*). Now that the
// Java throw is impossible, they are the same callbacks.
typedef LIBMATTI_MLA_Consumer_WithExceptions LIBMATTI_MLA_Consumer;
typedef LIBMATTI_MLA_BiConsumer_WithExceptions LIBMATTI_MLA_BiConsumer;
typedef LIBMATTI_MLA_Function_WithExceptions LIBMATTI_MLA_Function;
typedef LIBMATTI_MLA_Supplier_WithExceptions LIBMATTI_MLA_Supplier;

// Java: static <T,E> Consumer<T> rethrowConsumer(Consumer_WithExceptions<T,E> consumer)
LIBMATTI_MLA_Consumer *LIBMATTI_MLA_LambdaExceptionUtils_RethrowConsumer(
    LIBMATTI_MLA_Consumer_WithExceptions *consumer);
// Java: static <T,U,E> BiConsumer<T,U> rethrowBiConsumer(BiConsumer_WithExceptions<T,U,E> biConsumer)
LIBMATTI_MLA_BiConsumer *LIBMATTI_MLA_LambdaExceptionUtils_RethrowBiConsumer(
    LIBMATTI_MLA_BiConsumer_WithExceptions *biConsumer);
// Java: static <T,R,E> Function<T,R> rethrowFunction(Function_WithExceptions<T,R,E> function)
LIBMATTI_MLA_Function *LIBMATTI_MLA_LambdaExceptionUtils_RethrowFunction(
    LIBMATTI_MLA_Function_WithExceptions *function);
// Java: static <T,E> Supplier<T> rethrowSupplier(Supplier_WithExceptions<T,E> function)
LIBMATTI_MLA_Supplier *LIBMATTI_MLA_LambdaExceptionUtils_RethrowSupplier(
    LIBMATTI_MLA_Supplier_WithExceptions *function);
// Java: static void uncheck(Runnable_WithExceptions t)
void LIBMATTI_MLA_LambdaExceptionUtils_UncheckRunnable(LIBMATTI_MLA_Runnable_WithExceptions *t);
// Java: static <R,E> R uncheck(Supplier_WithExceptions<R,E> supplier)
void *LIBMATTI_MLA_LambdaExceptionUtils_UncheckSupplier(LIBMATTI_MLA_Supplier_WithExceptions *supplier);
// Java: static <T,R,E> R uncheck(Function_WithExceptions<T,R,E> function, T t)
void *LIBMATTI_MLA_LambdaExceptionUtils_UncheckFunction(LIBMATTI_MLA_Function_WithExceptions *function, void *value);

// Java: throwAsUnchecked(Exception) - cannot be expressed in C; documented only:
//       a callback reports failure through its own contract (return value).

#endif //MATTICRAFT_MODLAUNCHER_LAMBDAEXCEPTIONUTILS_H

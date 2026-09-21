// Port of cpw.mods.modlauncher.api.LambdaExceptionUtils.

#include "LambdaExceptionUtils.h"

// The Java wrappers build a new lambda that catches and rethrows unchecked.
// C has no checked exceptions, so every wrapper hands back the callback it was
// given; the call sites keep their own error contract.

LIBMATTI_MLA_Consumer *LIBMATTI_MLA_LambdaExceptionUtils_RethrowConsumer(
    LIBMATTI_MLA_Consumer_WithExceptions *consumer)
{
    return consumer;
}

LIBMATTI_MLA_BiConsumer *LIBMATTI_MLA_LambdaExceptionUtils_RethrowBiConsumer(
    LIBMATTI_MLA_BiConsumer_WithExceptions *biConsumer)
{
    return biConsumer;
}

LIBMATTI_MLA_Function *LIBMATTI_MLA_LambdaExceptionUtils_RethrowFunction(
    LIBMATTI_MLA_Function_WithExceptions *function)
{
    return function;
}

LIBMATTI_MLA_Supplier *LIBMATTI_MLA_LambdaExceptionUtils_RethrowSupplier(
    LIBMATTI_MLA_Supplier_WithExceptions *function)
{
    return function;
}

void LIBMATTI_MLA_LambdaExceptionUtils_UncheckRunnable(LIBMATTI_MLA_Runnable_WithExceptions *t)
{
    t->run(t);
}

void *LIBMATTI_MLA_LambdaExceptionUtils_UncheckSupplier(LIBMATTI_MLA_Supplier_WithExceptions *supplier)
{
    return supplier->get(supplier);
}

void *LIBMATTI_MLA_LambdaExceptionUtils_UncheckFunction(LIBMATTI_MLA_Function_WithExceptions *function, void *value)
{
    return function->apply(function, value);
}

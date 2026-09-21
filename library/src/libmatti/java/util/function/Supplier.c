// Port of java.util.function.Supplier.

#include "libmatti/java/util/function/Supplier.h"

// Java: T get()
void *LIBMATTI_JU_Supplier_Get(const LIBMATTI_JU_Supplier *supplier)
{
    return supplier->get(supplier->self);
}

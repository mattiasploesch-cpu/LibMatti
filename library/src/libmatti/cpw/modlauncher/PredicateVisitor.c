// Port of cpw.mods.modlauncher.PredicateVisitor.

#include "libmatti/cpw/modlauncher/PredicateVisitor.h"

int LIBMATTI_ML_PredicateVisitor_ApplyField(const LIBMATTI_ASMT_FieldNode *node,
                                            LIBMATTI_ML_PredicateVisitor_FieldTest predicate, void *userdata)
{
    // Java: result = fieldPredicate == null || fieldPredicate.test(...)
    if (predicate == NULL) return 1;
    return predicate(node->access, node->name, node->desc, node->signature, &node->value, userdata);
}

int LIBMATTI_ML_PredicateVisitor_ApplyMethod(const LIBMATTI_ASMT_MethodNode *node,
                                             LIBMATTI_ML_PredicateVisitor_MethodTest predicate, void *userdata)
{
    if (predicate == NULL) return 1;
    return predicate(node->access, node->name, node->desc, node->signature, node->exceptions, node->exceptionCount,
                     userdata);
}

int LIBMATTI_ML_PredicateVisitor_ApplyClass(const LIBMATTI_ASMT_ClassNode *node,
                                            LIBMATTI_ML_PredicateVisitor_ClassTest predicate, void *userdata)
{
    if (predicate == NULL) return 1;

    // Java visits the class, then every field and every method, and its
    // visitField/visitMethod overwrite the result with `true` when the matching
    // predicate is null. The C port evaluates only the class predicate, which is
    // what the caller of a class predicate expects.
    return predicate(node->version, node->access, node->name, node->signature, node->superName, node->interfaces,
                     node->interfaceCount, userdata);
}

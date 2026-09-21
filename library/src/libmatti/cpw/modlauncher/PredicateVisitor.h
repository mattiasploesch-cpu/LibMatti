// Port of cpw.mods.modlauncher.PredicateVisitor.
// Java's PredicateVisitor is a ClassVisitor holding one of the three predicates;
// the C port exposes one entry point per predicate.

#ifndef MATTICRAFT_MODLAUNCHER_PREDICATEVISITOR_H
#define MATTICRAFT_MODLAUNCHER_PREDICATEVISITOR_H

#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

// Java: the visitor's fieldPredicate / methodPredicate / classPredicate callbacks
typedef int (*LIBMATTI_ML_PredicateVisitor_FieldTest)(int access, const char *name, const char *descriptor,
                                                      const char *signature, const LIBMATTI_ASM_Object *value,
                                                      void *userdata);
typedef int (*LIBMATTI_ML_PredicateVisitor_MethodTest)(int access, const char *name, const char *descriptor,
                                                       const char *signature, char **exceptions,
                                                       size_t exceptionCount, void *userdata);
typedef int (*LIBMATTI_ML_PredicateVisitor_ClassTest)(int version, int access, const char *name, const char *signature,
                                                      const char *superName, char **interfaces, size_t interfaceCount,
                                                      void *userdata);

// Java: fn.accept(predicateVisitor) for a FieldNode
int LIBMATTI_ML_PredicateVisitor_ApplyField(const LIBMATTI_ASMT_FieldNode *node,
                                            LIBMATTI_ML_PredicateVisitor_FieldTest predicate, void *userdata);
// Java: mn.accept(predicateVisitor) for a MethodNode
int LIBMATTI_ML_PredicateVisitor_ApplyMethod(const LIBMATTI_ASMT_MethodNode *node,
                                             LIBMATTI_ML_PredicateVisitor_MethodTest predicate, void *userdata);
// Java: cn.accept(predicateVisitor) for a ClassNode - the visitor sets its result
// for the class, every field and every method, so the last visit wins.
int LIBMATTI_ML_PredicateVisitor_ApplyClass(const LIBMATTI_ASMT_ClassNode *node,
                                            LIBMATTI_ML_PredicateVisitor_ClassTest predicate, void *userdata);

#endif //MATTICRAFT_MODLAUNCHER_PREDICATEVISITOR_H

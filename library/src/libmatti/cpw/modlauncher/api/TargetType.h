// Port of cpw.mods.modlauncher.api.TargetType.

#ifndef MATTICRAFT_MODLAUNCHER_TARGETTYPE_H
#define MATTICRAFT_MODLAUNCHER_TARGETTYPE_H

#include <stddef.h>

// Java: public final class TargetType<T>
// Java identifies the node type by its Class; here the class name takes its place.
typedef struct LIBMATTI_MLA_TargetType
{
    const char *name;
    const char *nodeType;
} LIBMATTI_MLA_TargetType;

// Java: static final TargetType<ClassNode> PRE_CLASS / CLASS; TargetType<MethodNode> METHOD; TargetType<FieldNode> FIELD
const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_PreClass(void);
const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_Class(void);
const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_Method(void);
const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_Field(void);

// Java: static final TargetType<?>[] VALUES
const LIBMATTI_MLA_TargetType **LIBMATTI_MLA_TargetType_Values(size_t *count);

// Java: static TargetType<?> byName(String name)
const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_ByName(const char *name);

// Java: Class<T> getNodeType()
const char *LIBMATTI_MLA_TargetType_GetNodeType(const LIBMATTI_MLA_TargetType *type);

#endif //MATTICRAFT_MODLAUNCHER_TARGETTYPE_H

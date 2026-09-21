// Port of cpw.mods.modlauncher.api.TargetType.

#include "libmatti/cpw/modlauncher/api/TargetType.h"

#include <string.h>

static const LIBMATTI_MLA_TargetType PRE_CLASS = {"PRE_CLASS", "org.objectweb.asm.tree.ClassNode"};
static const LIBMATTI_MLA_TargetType CLASS = {"CLASS", "org.objectweb.asm.tree.ClassNode"};
static const LIBMATTI_MLA_TargetType METHOD = {"METHOD", "org.objectweb.asm.tree.MethodNode"};
static const LIBMATTI_MLA_TargetType FIELD = {"FIELD", "org.objectweb.asm.tree.FieldNode"};

const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_PreClass(void)
{
    return &PRE_CLASS;
}

const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_Class(void)
{
    return &CLASS;
}

const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_Method(void)
{
    return &METHOD;
}

const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_Field(void)
{
    return &FIELD;
}

const LIBMATTI_MLA_TargetType **LIBMATTI_MLA_TargetType_Values(size_t *count)
{
    static const LIBMATTI_MLA_TargetType *values[4];
    values[0] = &PRE_CLASS;
    values[1] = &CLASS;
    values[2] = &METHOD;
    values[3] = &FIELD;
    *count = 4;
    return values;
}

const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_TargetType_ByName(const char *name)
{
    size_t count = 0;
    const LIBMATTI_MLA_TargetType **values = LIBMATTI_MLA_TargetType_Values(&count);

    for (size_t i = 0; i < count; i++)
        if (strcmp(values[i]->name, name) == 0) return values[i];

    return NULL;
}

const char *LIBMATTI_MLA_TargetType_GetNodeType(const LIBMATTI_MLA_TargetType *type)
{
    return type->nodeType;
}

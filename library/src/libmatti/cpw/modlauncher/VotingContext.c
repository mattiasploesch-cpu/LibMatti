// Port of cpw.mods.modlauncher.VotingContext.

#include "libmatti/cpw/modlauncher/VotingContext.h"

#include "libmatti/cpw/modlauncher/PredicateVisitor.h"
#include "libmatti/org/objectweb/asm/tree/InsnList.h"
#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_ML_VotingContext *LIBMATTI_ML_VotingContext_New(const char *className, int classExists,
                                                         unsigned char *(*sha256)(void *userdata, size_t *length),
                                                         void *sha256Userdata,
                                                         LIBMATTI_MLA_ITransformerActivity *activities,
                                                         size_t activityCount, const char *reason)
{
    LIBMATTI_ML_VotingContext *context = calloc(1, sizeof(LIBMATTI_ML_VotingContext));
    context->className = strdup(className);
    context->classExists = classExists;
    context->sha256 = sha256;
    context->sha256Userdata = sha256Userdata;
    context->auditActivities = activities;
    context->auditActivityCount = activityCount;
    context->reason = strdup(reason);
    return context;
}

void LIBMATTI_ML_VotingContext_Free(LIBMATTI_ML_VotingContext *context)
{
    if (context == NULL) return;
    free(context->className);
    free(context->reason);
    free(context);
}

void LIBMATTI_ML_VotingContext_SetNode(LIBMATTI_ML_VotingContext *context, void *node)
{
    context->node = node;
}

// the ITransformerVotingContext contract

const char *LIBMATTI_MLA_ITransformerVotingContext_GetClassName(const LIBMATTI_MLA_ITransformerVotingContext *context)
{
    return context->className;
}

int LIBMATTI_MLA_ITransformerVotingContext_DoesClassExist(const LIBMATTI_MLA_ITransformerVotingContext *context)
{
    return context->classExists;
}

void LIBMATTI_MLA_ITransformerVotingContext_GetInitialClassSha256(
    const LIBMATTI_MLA_ITransformerVotingContext *context, unsigned char *out, size_t *outLength)
{
    // Java: return sha256.get()
    size_t length = 0;
    unsigned char *digest = context->sha256(context->sha256Userdata, &length);
    if (digest != NULL && out != NULL) memcpy(out, digest, length);
    *outLength = length;
}

LIBMATTI_MLA_ITransformerActivity *LIBMATTI_MLA_ITransformerVotingContext_GetAuditActivities(
    const LIBMATTI_MLA_ITransformerVotingContext *context, size_t *count)
{
    *count = context->auditActivityCount;
    return context->auditActivities;
}

const char *LIBMATTI_MLA_ITransformerVotingContext_GetReason(const LIBMATTI_MLA_ITransformerVotingContext *context)
{
    return context->reason;
}

int LIBMATTI_MLA_ITransformerVotingContext_ApplyFieldPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    int (*predicate)(int access, const char *name, const char *descriptor, const char *signature,
                     const LIBMATTI_ASM_Object *value, void *userdata),
    void *userdata)
{
    return LIBMATTI_ML_PredicateVisitor_ApplyField((const LIBMATTI_ASMT_FieldNode *)context->node, predicate, userdata);
}

int LIBMATTI_MLA_ITransformerVotingContext_ApplyMethodPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    int (*predicate)(int access, const char *name, const char *descriptor, const char *signature,
                     char **exceptions, size_t exceptionCount, void *userdata),
    void *userdata)
{
    return LIBMATTI_ML_PredicateVisitor_ApplyMethod((const LIBMATTI_ASMT_MethodNode *)context->node, predicate,
                                                    userdata);
}

int LIBMATTI_MLA_ITransformerVotingContext_ApplyClassPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    int (*predicate)(int version, int access, const char *name, const char *signature, const char *superName,
                     char **interfaces, size_t interfaceCount, void *userdata),
    void *userdata)
{
    return LIBMATTI_ML_PredicateVisitor_ApplyClass((const LIBMATTI_ASMT_ClassNode *)context->node, predicate,
                                                   userdata);
}

// Java: private Object[] toObjectArray(AbstractInsnNode)
static void insn_args(LIBMATTI_ASMT_AbstractInsnNode *node, LIBMATTI_ASM_Object *args, size_t *count)
{
    if (node->type == LIBMATTI_ASMT_METHOD_INSN)
    {
        LIBMATTI_ASMT_MethodInsnNode *method = (LIBMATTI_ASMT_MethodInsnNode *)node;
        args[0] = LIBMATTI_ASM_Object_OfString(method->name);
        args[1] = LIBMATTI_ASM_Object_OfString(method->desc);
        args[2] = LIBMATTI_ASM_Object_OfString(method->owner);
        args[3] = LIBMATTI_ASM_Object_OfInt(method->itf);
        *count = 4;
        return;
    }

    if (node->type == LIBMATTI_ASMT_FIELD_INSN)
    {
        LIBMATTI_ASMT_FieldInsnNode *field = (LIBMATTI_ASMT_FieldInsnNode *)node;
        args[0] = LIBMATTI_ASM_Object_OfString(field->name);
        args[1] = LIBMATTI_ASM_Object_OfString(field->desc);
        args[2] = LIBMATTI_ASM_Object_OfString(field->owner);
        *count = 3;
        return;
    }

    *count = 0;
}

int LIBMATTI_MLA_ITransformerVotingContext_ApplyInstructionPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    LIBMATTI_MLA_ITransformerVotingContext_InsnPredicate predicate, void *userdata)
{
    LIBMATTI_ASMT_MethodNode *method = (LIBMATTI_ASMT_MethodNode *)context->node;

    size_t insnCount = 0;
    LIBMATTI_ASMT_AbstractInsnNode **insns = LIBMATTI_ASM_InsnList_ToArray(&method->instructions, &insnCount);

    LIBMATTI_ASM_Object args[4];
    size_t argCount = 0;
    // Java: toObjectArray(insns[0]) - the original passes the first instruction
    if (insnCount > 0) insn_args(insns[0], args, &argCount);

    int result = 0;
    for (size_t i = 0; i < insnCount; i++)
    {
        if (predicate((int)i, LIBMATTI_ASM_AbstractInsnNode_GetOpcode(insns[i]), args, argCount, userdata)) result = 1;
    }

    for (size_t i = 0; i < argCount; i++) LIBMATTI_ASM_Object_Free(&args[i]);
    free(insns);

    return result;
}

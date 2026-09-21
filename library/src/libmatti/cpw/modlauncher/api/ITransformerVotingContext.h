// Port of cpw.mods.modlauncher.api.ITransformerVotingContext.

#ifndef MATTICRAFT_MODLAUNCHER_ITRANSFORMERVOTINGCONTEXT_H
#define MATTICRAFT_MODLAUNCHER_ITRANSFORMERVOTINGCONTEXT_H

#include "libmatti/cpw/modlauncher/api/ITransformerActivity.h"
#include "libmatti/org/objectweb/asm/Object.h"

#include <stddef.h>
// Java: ITransformerVotingContext.InsnPredicate.test(int, int, Object...)
typedef int (*LIBMATTI_MLA_ITransformerVotingContext_InsnPredicate)(int insnCount, int opcode,
                                                                   LIBMATTI_ASM_Object *args, size_t argCount,
                                                                   void *userdata);

// Java: interface ITransformerVotingContext
typedef struct LIBMATTI_MLA_ITransformerVotingContext LIBMATTI_MLA_ITransformerVotingContext;

// Java: String getClassName()
const char *LIBMATTI_MLA_ITransformerVotingContext_GetClassName(
    const LIBMATTI_MLA_ITransformerVotingContext *context);
// Java: boolean doesClassExist()
int LIBMATTI_MLA_ITransformerVotingContext_DoesClassExist(const LIBMATTI_MLA_ITransformerVotingContext *context);
// Java: byte[] getInitialClassSha256()
void LIBMATTI_MLA_ITransformerVotingContext_GetInitialClassSha256(
    const LIBMATTI_MLA_ITransformerVotingContext *context, unsigned char *out, size_t *outLength);
// Java: List<ITransformerActivity> getAuditActivities()
LIBMATTI_MLA_ITransformerActivity *LIBMATTI_MLA_ITransformerVotingContext_GetAuditActivities(
    const LIBMATTI_MLA_ITransformerVotingContext *context, size_t *count);
// Java: String getReason()
const char *LIBMATTI_MLA_ITransformerVotingContext_GetReason(const LIBMATTI_MLA_ITransformerVotingContext *context);

// Java: the four predicate appliers (see PredicateVisitor)
int LIBMATTI_MLA_ITransformerVotingContext_ApplyFieldPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    int (*predicate)(int access, const char *name, const char *descriptor, const char *signature,
                     const LIBMATTI_ASM_Object *value, void *userdata),
    void *userdata);
int LIBMATTI_MLA_ITransformerVotingContext_ApplyMethodPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    int (*predicate)(int access, const char *name, const char *descriptor, const char *signature,
                     char **exceptions, size_t exceptionCount, void *userdata),
    void *userdata);
int LIBMATTI_MLA_ITransformerVotingContext_ApplyClassPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    int (*predicate)(int version, int access, const char *name, const char *signature, const char *superName,
                     char **interfaces, size_t interfaceCount, void *userdata),
    void *userdata);
int LIBMATTI_MLA_ITransformerVotingContext_ApplyInstructionPredicate(
    LIBMATTI_MLA_ITransformerVotingContext *context,
    LIBMATTI_MLA_ITransformerVotingContext_InsnPredicate predicate, void *userdata);

#endif //MATTICRAFT_MODLAUNCHER_ITRANSFORMERVOTINGCONTEXT_H

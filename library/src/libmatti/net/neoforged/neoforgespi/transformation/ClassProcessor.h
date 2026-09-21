// Port of net.neoforged.neoforgespi.transformation.ClassProcessor.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_CLASSPROCESSOR_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_CLASSPROCESSOR_H

#include "libmatti/net/neoforged/neoforgespi/transformation/BytecodeProvider.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ProcessorName.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/SimpleTransformationContext.h"
#include "libmatti/org/objectweb/asm/Type.h"
#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

#include <stddef.h>

// Java: String GENERATED_PACKAGE_MODULE
#define LIBMATTI_NEOFORGESPI_ClassProcessor_GENERATED_PACKAGE_MODULE "net.neoforged.fml.generated"

// Java: enum OrderingHint { EARLY, DEFAULT, LATE }
typedef enum
{
    LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint_EARLY = 0,
    LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint_DEFAULT,
    LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint_LATE
} LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint;

// Java: enum ComputeFlags { NO_REWRITE, SIMPLE_REWRITE, COMPUTE_MAXS, COMPUTE_FRAMES }
typedef enum
{
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE = 0,
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_SIMPLE_REWRITE,
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_MAXS,
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES
} LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags;

// Java: public ComputeFlags max(ComputeFlags other)
LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_Max(
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags flags,
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags other);

// Java: record SelectionContext(Type type, boolean empty)
typedef struct LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext
{
    const LIBMATTI_ASM_Type *type;
    int empty;
} LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext;

// Java: final class TransformationContext implements SimpleTransformationContext
typedef struct LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext
{
    // Java: implements SimpleTransformationContext
    LIBMATTI_NEOFORGESPI_SimpleTransformationContext simple;

    const LIBMATTI_ASM_Type *type;
    LIBMATTI_ASMT_ClassNode *node;
    int empty;
    // Java: BiConsumer<String, String[]> auditTrail
    void (*auditTrail)(const char *activity, const char *const *context, size_t contextCount, void *userdata);
    void *auditTrailUserdata;
    // Java: Supplier<byte[]> initialSha256
    unsigned char *(*initialSha256)(void *userdata, size_t *length);
    void *initialSha256Userdata;
} LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext;

// Java: public TransformationContext(Type, ClassNode, boolean, BiConsumer, Supplier)
LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_New(
    const LIBMATTI_ASM_Type *type, LIBMATTI_ASMT_ClassNode *node, int empty,
    void (*auditTrail)(const char *activity, const char *const *context, size_t contextCount, void *userdata),
    void *auditTrailUserdata, unsigned char *(*initialSha256)(void *userdata, size_t *length),
    void *initialSha256Userdata);
void LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Free(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);

// Java: public Type type()
const LIBMATTI_ASM_Type *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Type(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);
// Java: public ClassNode node()
LIBMATTI_ASMT_ClassNode *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Node(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);
// Java: public boolean empty()
int LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Empty(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);
// Java: public void audit(String activity, String... context)
void LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Audit(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context, const char *activity,
    const char *const *arguments, size_t argumentCount);
// Java: public byte[] initialSha256(); caller frees
unsigned char *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_InitialSha256(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context, size_t *length);
// Java: the upcast to SimpleTransformationContext
LIBMATTI_NEOFORGESPI_SimpleTransformationContext *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Simple(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);

// Java: record AfterProcessingContext(Type type)
typedef struct LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext
{
    const LIBMATTI_ASM_Type *type;
} LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext;

// Java: interface ClassProcessor
typedef struct LIBMATTI_NEOFORGESPI_ClassProcessor LIBMATTI_NEOFORGESPI_ClassProcessor;

// Java: record LinkContext(SequencedMap<ProcessorName, ClassProcessor> processors, BytecodeProvider bytecodeProvider)
typedef struct LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext
{
    // Java: SequencedMap<ProcessorName, ClassProcessor> processors - the insertion order
    const LIBMATTI_NEOFORGESPI_ProcessorName **processorNames;
    LIBMATTI_NEOFORGESPI_ClassProcessor **processors;
    size_t processorCount;
    LIBMATTI_NEOFORGESPI_BytecodeProvider *bytecodeProvider;
} LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext;

// Java: interface ClassProcessor - NULL is a Java default method
struct LIBMATTI_NEOFORGESPI_ClassProcessor
{
    // Java: ProcessorName name()
    LIBMATTI_NEOFORGESPI_ProcessorName *(*name)(LIBMATTI_NEOFORGESPI_ClassProcessor *self);
    // Java: default Set<ProcessorName> runsBefore(); caller frees the array
    const LIBMATTI_NEOFORGESPI_ProcessorName **(*runsBefore)(LIBMATTI_NEOFORGESPI_ClassProcessor *self, size_t *count);
    // Java: default Set<ProcessorName> runsAfter(); caller frees the array
    const LIBMATTI_NEOFORGESPI_ProcessorName **(*runsAfter)(LIBMATTI_NEOFORGESPI_ClassProcessor *self, size_t *count);
    // Java: default Set<String> generatesPackages(); caller frees the array
    const char **(*generatesPackages)(LIBMATTI_NEOFORGESPI_ClassProcessor *self, size_t *count);
    // Java: default OrderingHint orderingHint()
    LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint (*orderingHint)(LIBMATTI_NEOFORGESPI_ClassProcessor *self);
    // Java: boolean handlesClass(SelectionContext context)
    int (*handlesClass)(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                        const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context);
    // Java: ComputeFlags processClass(TransformationContext context)
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags (*processClass)(
        LIBMATTI_NEOFORGESPI_ClassProcessor *self, LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);
    // Java: default void afterProcessing(AfterProcessingContext context)
    void (*afterProcessing)(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                            const LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext *context);
    // Java: default void link(LinkContext context)
    void (*link)(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                 const LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext *context);
};

// Java: the interface methods, dispatched to the implementing vtable (including defaults)
LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessor_Name(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor);
const LIBMATTI_NEOFORGESPI_ProcessorName **LIBMATTI_NEOFORGESPI_ClassProcessor_RunsBefore(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor, size_t *count);
const LIBMATTI_NEOFORGESPI_ProcessorName **LIBMATTI_NEOFORGESPI_ClassProcessor_RunsAfter(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor, size_t *count);
const char **LIBMATTI_NEOFORGESPI_ClassProcessor_GeneratesPackages(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor, size_t *count);
LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint LIBMATTI_NEOFORGESPI_ClassProcessor_GetOrderingHint(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor);
int LIBMATTI_NEOFORGESPI_ClassProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context);
LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_ClassProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);
void LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessing(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext *context);
void LIBMATTI_NEOFORGESPI_ClassProcessor_Link(LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
                                             const LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext *context);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_CLASSPROCESSOR_H

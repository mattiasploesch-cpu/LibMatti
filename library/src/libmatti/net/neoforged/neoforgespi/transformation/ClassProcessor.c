// Port of net.neoforged.neoforgespi.transformation.ClassProcessor.

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

#include <stddef.h>
#include <stdlib.h>

LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_Max(
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags flags,
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags other)
{
    // Java: return this.ordinal() > other.ordinal() ? this : other
    return flags > other ? flags : other;
}

// Java: the TransformationContext implementation of SimpleTransformationContext, recovered from the
// embedded (first) member
static LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *transformation_context(
    LIBMATTI_NEOFORGESPI_SimpleTransformationContext *self)
{
    return (LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *) self;
}

static const LIBMATTI_ASM_Type *transformation_context_type(
    LIBMATTI_NEOFORGESPI_SimpleTransformationContext *self)
{
    return transformation_context(self)->type;
}

static int transformation_context_empty(LIBMATTI_NEOFORGESPI_SimpleTransformationContext *self)
{
    return transformation_context(self)->empty;
}

static unsigned char *transformation_context_initial_sha256(LIBMATTI_NEOFORGESPI_SimpleTransformationContext *self,
                                                            size_t *length)
{
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context = transformation_context(self);
    return context->initialSha256(context->initialSha256Userdata, length);
}

LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_New(
    const LIBMATTI_ASM_Type *type, LIBMATTI_ASMT_ClassNode *node, int empty,
    void (*auditTrail)(const char *activity, const char *const *context, size_t contextCount, void *userdata),
    void *auditTrailUserdata, unsigned char *(*initialSha256)(void *userdata, size_t *length),
    void *initialSha256Userdata)
{
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context =
        calloc(1, sizeof(LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext));
    context->simple.type = transformation_context_type;
    context->simple.empty = transformation_context_empty;
    context->simple.initialSha256 = transformation_context_initial_sha256;
    context->type = type;
    context->node = node;
    context->empty = empty;
    context->auditTrail = auditTrail;
    context->auditTrailUserdata = auditTrailUserdata;
    context->initialSha256 = initialSha256;
    context->initialSha256Userdata = initialSha256Userdata;
    return context;
}

void LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Free(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    free(context);
}

const LIBMATTI_ASM_Type *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Type(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    return context->type;
}

LIBMATTI_ASMT_ClassNode *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Node(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    return context->node;
}

int LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Empty(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    return context->empty;
}

void LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Audit(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context, const char *activity,
    const char *const *arguments, size_t argumentCount)
{
    context->auditTrail(activity, arguments, argumentCount, context->auditTrailUserdata);
}

unsigned char *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_InitialSha256(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context, size_t *length)
{
    return context->initialSha256(context->initialSha256Userdata, length);
}

LIBMATTI_NEOFORGESPI_SimpleTransformationContext *LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Simple(
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    return &context->simple;
}

// Java: default Set<ProcessorName> runsBefore() { return Set.of(); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **default_runs_before(size_t *count)
{
    *count = 0;
    return NULL;
}

// Java: default Set<ProcessorName> runsAfter() { return Set.of(ClassProcessorIds.COMPUTING_FRAMES); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **default_runs_after(size_t *count)
{
    static const LIBMATTI_NEOFORGESPI_ProcessorName *names[1];
    names[0] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES();
    *count = 1;
    return names;
}

// Java: default Set<String> generatesPackages() { return Set.of(); }
static const char **default_generates_packages(size_t *count)
{
    *count = 0;
    return NULL;
}

LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessor_Name(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor)
{
    return processor->name(processor);
}

const LIBMATTI_NEOFORGESPI_ProcessorName **LIBMATTI_NEOFORGESPI_ClassProcessor_RunsBefore(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor, size_t *count)
{
    if (processor->runsBefore == NULL) return default_runs_before(count);
    return processor->runsBefore(processor, count);
}

const LIBMATTI_NEOFORGESPI_ProcessorName **LIBMATTI_NEOFORGESPI_ClassProcessor_RunsAfter(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor, size_t *count)
{
    if (processor->runsAfter == NULL) return default_runs_after(count);
    return processor->runsAfter(processor, count);
}

const char **LIBMATTI_NEOFORGESPI_ClassProcessor_GeneratesPackages(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor, size_t *count)
{
    if (processor->generatesPackages == NULL) return default_generates_packages(count);
    return processor->generatesPackages(processor, count);
}

LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint LIBMATTI_NEOFORGESPI_ClassProcessor_GetOrderingHint(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor)
{
    // Java: default OrderingHint orderingHint() { return OrderingHint.DEFAULT; }
    if (processor->orderingHint == NULL) return LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint_DEFAULT;
    return processor->orderingHint(processor);
}

int LIBMATTI_NEOFORGESPI_ClassProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    return processor->handlesClass(processor, context);
}

LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_ClassProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    return processor->processClass(processor, context);
}

void LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessing(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext *context)
{
    // Java: default void afterProcessing(AfterProcessingContext context) {}
    if (processor->afterProcessing == NULL) return;
    processor->afterProcessing(processor, context);
}

void LIBMATTI_NEOFORGESPI_ClassProcessor_Link(LIBMATTI_NEOFORGESPI_ClassProcessor *processor,
                                             const LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext *context)
{
    // Java: default void link(LinkContext context) {}
    if (processor->link == NULL) return;
    processor->link(processor, context);
}

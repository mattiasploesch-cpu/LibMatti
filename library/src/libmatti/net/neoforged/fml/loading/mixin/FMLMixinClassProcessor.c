// Ports of net.neoforged.fml.loading.mixin.FMLMixinClassProcessor and
// net.neoforged.fml.loading.mixin.FMLMixinGeneratingClassProcessor.
// Java's processors drive the bytecode transformer; the port's native backend has no bytecode,
// so handlesClass/processClass resolve over the hook table's registered targets - a class is
// "handled" when a hook descriptor targets it, and "processed" when the chain runs.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinClassProcessor.h"

#include "libmatti/matti/mixin/MixinHookTable.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

#include <stdio.h>
#include "libmatti/net/neoforged/fml/loading/mixin/FMLAuditTrail.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLClassTracker.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinService.h"
#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdlib.h>
#include <string.h>

// Java: context.type().getClassName()
static const char *type_class_name(const LIBMATTI_ASM_Type *type)
{
    char *name = LIBMATTI_ASM_Type_GetClassName((LIBMATTI_ASM_Type *) type);
    // The port's Type keeps a stable name; a stack copy suffices for the lookup calls below.
    static char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s", name);
    free(name);
    return buffer;
}

// Java: public FMLMixinClassProcessor(FMLMixinService service) { this.auditTrail = ...;
//        this.classTracker = ...; this.transformer = service.getMixinTransformer(); }
// Java: name() { return ClassProcessorIds.MIXIN; }
static LIBMATTI_NEOFORGESPI_ProcessorName *mixin_processor_name(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return (LIBMATTI_NEOFORGESPI_ProcessorName *) LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN();
}

// Java: name() { return ClassProcessorIds.MIXIN_FRAME_CONTEXT; }
static LIBMATTI_NEOFORGESPI_ProcessorName *generating_processor_name(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return (LIBMATTI_NEOFORGESPI_ProcessorName *) LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN_FRAME_CONTEXT();
}

// Java: public Set<ProcessorName> runsBefore() { return Set.of(COMPUTING_FRAMES, MIXIN); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **generating_runs_before(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                                                                         size_t *count)
{
    (void) self;
    static const LIBMATTI_NEOFORGESPI_ProcessorName *RUNS_BEFORE[2];
    RUNS_BEFORE[0] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES();
    RUNS_BEFORE[1] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN();
    *count = 2;
    return RUNS_BEFORE;
}

// Java: public Set<ProcessorName> runsAfter() { return Set.of(); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **generating_runs_after(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                                                                        size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

// Java: public Set<String> generatesPackages() { return Set.of(ArgsClassGenerator.SYNTHETIC_PACKAGE); }
// The synthetic package belongs to the bytecode transformer; the port generates nothing.
static const char **processor_generates_packages(LIBMATTI_NEOFORGESPI_ClassProcessor *self, size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

// Java: default OrderingHint orderingHint() { return OrderingHint.DEFAULT; }
static LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint processor_ordering_hint(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint_DEFAULT;
}

// Java: public boolean handlesClass(SelectionContext context)
//       { String name = context.type().getClassName();
//         if (this.classTracker.isInvalidClass(name)) throw new NoClassDefFoundError(...);
//         if (!context.empty() && processesClass(context.type())) return true; ... }
// The port's handlesClass: the hook table's target lookup (a class is handled when a mixin
// targets it); invalid classes stay refused.
static int mixin_processor_handles_class(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                                         const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    (void) self;
    if (context->type == NULL) return 0;

    // Java: String name = context.type().getClassName();
    const char *name = type_class_name(context->type);

    // Java: if (this.classTracker.isInvalidClass(name)) throw new NoClassDefFoundError(...);
    if (LIBMATTI_FML_FMLClassTracker_IsInvalidClass(name)) return 0;

    // Java: if (!context.empty()) { if (processesClass(context.type())) return true; }
    if (!context->empty)
    {
        // Java: processesClass -> transformer.couldTransformClass(environment, className)
        return LIBMATTI_MIXIN_Lookup(LIBMATTI_MIXIN_HookTable_Default(), name) != NULL;
    }

    return 0;
}

// Java: public ComputeFlags processClass(TransformationContext context)
//       { this.auditTrail.setConsumer(...); ... return transformer.transformClass(...) ? COMPUTE_FRAMES : NO_REWRITE; }
static LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags mixin_processor_process_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self, LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    (void) self;
    if (context->type == NULL) return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;

    // Java: this.auditTrail.setConsumer(classType.getClassName(), context::audit);
    const char *className = type_class_name(context->type);
    LIBMATTI_SP_IMixinAuditTrail *auditTrail = LIBMATTI_FML_FMLAuditTrail_Instance();
    auditTrail->setConsumer(auditTrail->self, className,
                            (void (*)(const char *, const char **, size_t, void *)) context->auditTrail,
                            context->auditTrailUserdata);

    // Java: transformer.transformClass(environment, className, classNode)
    // The native backend runs the hook chain for the target in the TransformingClassLoader's
    // transform step, so the processor marks the rewrite requirement Java's COMPUTE_FRAMES carries.
    return LIBMATTI_MIXIN_Lookup(LIBMATTI_MIXIN_HookTable_Default(), type_class_name(context->type)) != NULL
               ? LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES
               : LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}

// Java: public void afterProcessing(AfterProcessingContext context)
//       { this.classTracker.addLoadedClass(context.type().getClassName()); }
static void mixin_processor_after_processing(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self, const LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext *context)
{
    (void) self;
    if (context->type != NULL) LIBMATTI_FML_FMLClassTracker_AddLoadedClass(type_class_name(context->type));
}

// Java: default void link(LinkContext context) {}
static void processor_no_link(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                              const LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext *context)
{
    (void) self;
    (void) context;
}

// Java: public boolean handlesClass(SelectionContext context)
//       { if (registry == null) return false; return generatesClass(registry, context.type()); }
static int generating_processor_handles_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self, const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    (void) self;
    (void) context;
    // The port's synthetic class registry has no entries (the bytecode transformer is external)
    return 0;
}

// Java: public ComputeFlags processClass(TransformationContext context) { ... return NO_REWRITE; }
static LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags generating_processor_process_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self, LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    (void) self;
    (void) context;
    return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}

// Java: public void link(LinkContext context)
//       { this.service.setBytecodeProvider(new FMLClassBytecodeProvider(context.bytecodeProvider(), this.service)); }
static void generating_processor_link(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                                      const LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext *context)
{
    (void) self;
    // Java: setBytecodeProvider(...) - the port's service keeps the provider for close()
    LIBMATTI_FML_FMLMixinService_SetBytecodeProvider(
        context != NULL ? (void *) context->bytecodeProvider : NULL);
}

// Java: public FMLMixinClassProcessor(FMLMixinService service)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLMixinClassProcessor_New(void)
{
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor = calloc(1, sizeof(*processor));
    processor->name = mixin_processor_name;
    processor->runsBefore = NULL; // Java: default Set.of()
    processor->runsAfter = NULL;  // Java: default Set.of()
    processor->generatesPackages = processor_generates_packages;
    processor->orderingHint = processor_ordering_hint;
    processor->handlesClass = mixin_processor_handles_class;
    processor->processClass = mixin_processor_process_class;
    processor->afterProcessing = mixin_processor_after_processing;
    processor->link = processor_no_link;
    return processor;
}

// Java: public FMLMixinGeneratingClassProcessor(FMLMixinService service)
LIBMATTI_NEOFORGESPI_ClassProcessor *LIBMATTI_FML_FMLMixinGeneratingClassProcessor_New(void)
{
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor = calloc(1, sizeof(*processor));
    processor->name = generating_processor_name;
    processor->runsBefore = generating_runs_before;
    processor->runsAfter = generating_runs_after;
    processor->generatesPackages = processor_generates_packages;
    processor->orderingHint = processor_ordering_hint;
    processor->handlesClass = generating_processor_handles_class;
    processor->processClass = generating_processor_process_class;
    processor->afterProcessing = NULL; // Java: default no-op
    processor->link = generating_processor_link;
    return processor;
}

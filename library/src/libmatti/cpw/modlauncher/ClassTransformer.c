// Port of cpw.mods.modlauncher.ClassTransformer.

#include "libmatti/cpw/modlauncher/ClassTransformer.h"

#include "LogManager.h"
#include "LogMarkers.h"
#include "libmatti/cpw/modlauncher/TransformerClassWriter.h"
#include "libmatti/cpw/modlauncher/TransformerHolder.h"
#include "libmatti/cpw/modlauncher/TransformerVote.h"
#include "libmatti/cpw/modlauncher/VoteDeadlockException.h"
#include "libmatti/cpw/modlauncher/VoteRejectedException.h"
#include "libmatti/cpw/modlauncher/VotingContext.h"
#include "libmatti/java/security/MessageDigest.h"
#include "libmatti/org/objectweb/asm/ClassReader.h"
#include "libmatti/org/objectweb/asm/ClassWriter.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private MessageDigest getSha256()
static LIBMATTI_JS_MessageDigest *get_sha256(void)
{
    return LIBMATTI_JS_MessageDigest_GetInstance("SHA-256");
}

// Java: Supplier<byte[]> sha256
typedef struct
{
    const unsigned char *data;
    size_t length;
} DigestSource;

static unsigned char *digest_source(void *userdata, size_t *length)
{
    DigestSource *source = userdata;
    LIBMATTI_JS_MessageDigest *digest = get_sha256();
    unsigned char *result = LIBMATTI_JS_MessageDigest_Digest(digest, source->data, source->length, length);
    LIBMATTI_JS_MessageDigest_Free(digest);
    return result;
}

static char *to_internal_name(const char *className)
{
    char *result = strdup(className);
    for (char *c = result; *c != '\0'; c++) if (*c == '.') *c = '/';
    return result;
}

// Java: private <T> TransformerVote<T> gatherVote(ITransformer<T> transformer, VotingContext context)
static LIBMATTI_ML_TransformerVote gather_vote(LIBMATTI_MLA_ITransformer *transformer,
                                               LIBMATTI_MLA_ITransformerVotingContext *context)
{
    // Java: new TransformerVote<>(transformer.castVote(context), transformer)
    return LIBMATTI_ML_TransformerVote_New(LIBMATTI_MLA_ITransformer_CastVote(transformer, context), transformer);
}

// Java: private <T> T performVote(List<ITransformer<T>> transformers, T node, VotingContext context)
static void *perform_vote(LIBMATTI_ML_ClassTransformer *self, LIBMATTI_MLA_ITransformer **transformers,
                          size_t transformerCount, void *node, LIBMATTI_ML_VotingContext *context,
                          const char *nodeTypeName)
{
    // Java mutates the list it was handed; the C port works on a copy
    LIBMATTI_MLA_ITransformer **voters = malloc(sizeof(*voters) * (transformerCount > 0 ? transformerCount : 1));
    memcpy(voters, transformers, sizeof(*voters) * transformerCount);
    size_t voterCount = transformerCount;

    // Java sets the node once before the loop; the port keeps it in sync after
    // every transform so that predicates always see the current node.
    LIBMATTI_ML_VotingContext_SetNode(context, node);

    while (voterCount > 0)
    {
        LIBMATTI_ML_TransformerVote *votes = malloc(sizeof(*votes) * voterCount);
        for (size_t i = 0; i < voterCount; i++) votes[i] = gather_vote(voters[i], context);

        // Java: results = voteResultStream.collect(Collectors.groupingBy(TransformerVote::getResult))
        LIBMATTI_MLA_ITransformer *yes = NULL;
        int rejected = 0;
        int hasDefer = 0;
        LIBMATTI_MLA_ITransformer **noVoters = NULL;
        size_t noCount = 0;

        for (size_t i = 0; i < voterCount; i++)
        {
            switch (LIBMATTI_ML_TransformerVote_GetResult(&votes[i]))
            {
            case LIBMATTI_MLA_VOTE_YES:
                if (yes == NULL) yes = LIBMATTI_ML_TransformerVote_GetTransformer(&votes[i]);
                break;
            case LIBMATTI_MLA_VOTE_REJECT:
                rejected = 1;
                break;
            case LIBMATTI_MLA_VOTE_NO:
                noVoters = realloc(noVoters, sizeof(*noVoters) * (noCount + 1));
                noVoters[noCount++] = LIBMATTI_ML_TransformerVote_GetTransformer(&votes[i]);
                break;
            default:
                hasDefer = 1;
                break;
            }
        }

        if (rejected)
        {
            // Java: throw new VoteRejectedException(results.get(REJECT), node.getClass())
            LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                     "Transformer rejected the current state of {}", nodeTypeName);
            free(noVoters);
            free(votes);
            free(voters);
            return node;
        }

        // Java: remove all the "NO" voters - they don't wish to participate in further voting rounds
        for (size_t i = 0; i < noCount; i++)
        {
            for (size_t v = 0; v < voterCount; v++)
            {
                if (voters[v] != noVoters[i]) continue;
                for (size_t s = v + 1; s < voterCount; s++) voters[s - 1] = voters[s];
                voterCount--;
                break;
            }
        }
        free(noVoters);

        if (yes != NULL)
        {
            // Java: node = transformer.transform(node, context); auditTrail.addTransformerAuditTrail(...)
            node = LIBMATTI_MLA_ITransformer_Transform(yes, node, context);
            LIBMATTI_ML_TransformerAuditTrail_AddTransformerAuditTrail(
                self->auditTrail, LIBMATTI_MLA_ITransformerVotingContext_GetClassName(context),
                LIBMATTI_ML_TransformerHolder_Owner((LIBMATTI_ML_TransformerHolder *)yes), yes);

            for (size_t v = 0; v < voterCount; v++)
            {
                if (voters[v] != yes) continue;
                for (size_t s = v + 1; s < voterCount; s++) voters[s - 1] = voters[s];
                voterCount--;
                break;
            }

            LIBMATTI_ML_VotingContext_SetNode(context, node);
            free(votes);
            continue;
        }

        if (hasDefer)
        {
            // Java: throw new VoteDeadlockException(results.get(DEFER), node.getClass())
            LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                     "Transformer vote deadlock on {}", nodeTypeName);
            free(votes);
            free(voters);
            return node;
        }

        free(votes);
    }

    free(voters);
    return node;
}

LIBMATTI_ML_ClassTransformer *LIBMATTI_ML_ClassTransformer_New(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_TransformingClassLoader *transformingClassLoader)
{
    return LIBMATTI_ML_ClassTransformer_NewWithTrail(transformStore, pluginHandler, transformingClassLoader,
                                                     LIBMATTI_ML_TransformerAuditTrail_New());
}

LIBMATTI_ML_ClassTransformer *LIBMATTI_ML_ClassTransformer_NewWithTrail(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_TransformingClassLoader *transformingClassLoader, LIBMATTI_ML_TransformerAuditTrail *auditTrail)
{
    LIBMATTI_ML_ClassTransformer *transformer = calloc(1, sizeof(LIBMATTI_ML_ClassTransformer));
    transformer->transformers = transformStore;
    transformer->pluginHandler = pluginHandler;
    transformer->transformingClassLoader = transformingClassLoader;
    transformer->auditTrail = auditTrail;
    return transformer;
}

void LIBMATTI_ML_ClassTransformer_Free(LIBMATTI_ML_ClassTransformer *transformer)
{
    free(transformer);
}

LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_ClassTransformer_GetTransformingClassLoader(
    const LIBMATTI_ML_ClassTransformer *transformer)
{
    return transformer->transformingClassLoader;
}

static int set_is_empty(const LIBMATTI_ML_LaunchPluginTransformerSet *set)
{
    return set->beforeCount == 0 && set->afterCount == 0;
}

unsigned char *LIBMATTI_ML_ClassTransformer_Transform(LIBMATTI_ML_ClassTransformer *transformer,
                                                      const unsigned char *inputClass, size_t inputLength,
                                                      const char *className, const char *reason, size_t *outLength)
{
    // Java: final String internalName = className.replace('.', '/'); final Type classDesc = Type.getObjectType(internalName)
    char *internalName = to_internal_name(className);
    LIBMATTI_ASM_Type *classDesc = LIBMATTI_ASM_Type_GetObjectType(internalName);

    // Java: final EnumMap<Phase, List<ILaunchPluginService>> launchPluginTransformerSet =
    //           pluginHandler.computeLaunchPluginTransformerSet(classDesc, inputClass.length == 0, reason, auditTrail)
    LIBMATTI_ML_LaunchPluginTransformerSet launchPluginTransformerSet =
        LIBMATTI_ML_LaunchPluginHandler_ComputeLaunchPluginTransformerSet(transformer->pluginHandler, classDesc,
                                                                          inputLength == 0, reason,
                                                                          transformer->auditTrail);

    const int needsTransforming = LIBMATTI_ML_TransformStore_NeedsTransforming(transformer->transformers,
                                                                               internalName);
    if (!needsTransforming && set_is_empty(&launchPluginTransformerSet))
    {
        // Java: return inputClass
        unsigned char *result = malloc(inputLength > 0 ? inputLength : 1);
        if (inputLength > 0) memcpy(result, inputClass, inputLength);
        *outLength = inputLength;
        free(launchPluginTransformerSet.before);
        free(launchPluginTransformerSet.after);
        free(internalName);
        LIBMATTI_ASM_Type_Free(classDesc);
        return result;
    }

    // Java: ClassNode clazz = new ClassNode(Opcodes.ASM9)
    LIBMATTI_ASMT_ClassNode *clazz = LIBMATTI_ASMT_ClassNode_New();
    DigestSource digestSource = {inputClass, inputLength};
    int empty;

    if (inputLength > 0)
    {
        // Java: new ClassReader(inputClass).accept(clazz, ClassReader.EXPAND_FRAMES)
        LIBMATTI_ASM_ClassReader *reader = LIBMATTI_ASM_ClassReader_New(inputClass, inputLength,
                                                                        LIBMATTI_ASM_EXPAND_FRAMES);
        LIBMATTI_ASM_ClassReader_Accept(reader, clazz);
        LIBMATTI_ASM_ClassReader_Free(reader);
        empty = 0;
    }
    else
    {
        // Java: clazz.name = classDesc.getInternalName(); clazz.version = 52; clazz.superName = "java/lang/Object"
        clazz->name = strdup(internalName);
        clazz->version = 52;
        clazz->superName = strdup("java/lang/Object");
        empty = 1;
    }

    // Java: auditTrail.addReason(classDesc.getClassName(), reason)
    char *classNameString = LIBMATTI_ASM_Type_GetClassName(classDesc);
    LIBMATTI_ML_TransformerAuditTrail_AddReason(transformer->auditTrail, classNameString, reason);
    free(classNameString);

    // Java: final int preFlags = pluginHandler.offerClassNodeToPlugins(Phase.BEFORE, set.getOrDefault(BEFORE,List.of()), clazz, classDesc, auditTrail, reason)
    int preFlags = LIBMATTI_ML_LaunchPluginHandler_OfferClassNodeToPlugins(
        transformer->pluginHandler, LIBMATTI_MLS_PHASE_BEFORE, launchPluginTransformerSet.before,
        launchPluginTransformerSet.beforeCount, clazz, classDesc, transformer->auditTrail, reason);

    if (preFlags == LIBMATTI_MLS_COMPUTE_NO_REWRITE && !needsTransforming &&
        launchPluginTransformerSet.afterCount == 0)
    {
        // Java: shortcut if there's no further work to do
        unsigned char *result = malloc(inputLength > 0 ? inputLength : 1);
        if (inputLength > 0) memcpy(result, inputClass, inputLength);
        *outLength = inputLength;
        LIBMATTI_ASMT_ClassNode_Free(clazz);
        free(launchPluginTransformerSet.before);
        free(launchPluginTransformerSet.after);
        free(internalName);
        LIBMATTI_ASM_Type_Free(classDesc);
        return result;
    }

    if (needsTransforming)
    {
        size_t activityCount = 0;
        LIBMATTI_MLA_ITransformerActivity *activities =
            LIBMATTI_MLA_ITransformerAuditTrail_GetActivityFor(
                (LIBMATTI_MLA_ITransformerAuditTrail *)transformer->auditTrail, className, &activityCount);

        // Java: VotingContext context = new VotingContext(className, empty, digest, auditTrail.getActivityFor(className), reason)
        LIBMATTI_ML_VotingContext *context = LIBMATTI_ML_VotingContext_New(className, empty, digest_source,
                                                                           &digestSource, activities, activityCount,
                                                                           reason);

        // Java: clazz = performVote(preClassTransformers, clazz, context)
        size_t classTransformerCount = 0;
        LIBMATTI_MLA_ITransformer **classTransformers = LIBMATTI_ML_TransformStore_GetTransformersForClass(
            transformer->transformers, className, LIBMATTI_MLA_TargetType_PreClass(), &classTransformerCount);
        clazz = perform_vote(transformer, classTransformers, classTransformerCount, clazz, context, "ClassNode");

        // Java: for (FieldNode field : clazz.fields) fieldList.add(performVote(fieldTransformers, field, context))
        for (size_t i = 0; i < clazz->fieldCount; i++)
        {
            size_t fieldTransformerCount = 0;
            LIBMATTI_MLA_ITransformer **fieldTransformers = LIBMATTI_ML_TransformStore_GetTransformersForField(
                transformer->transformers, className, clazz->fields[i], &fieldTransformerCount);
            clazz->fields[i] = perform_vote(transformer, fieldTransformers, fieldTransformerCount, clazz->fields[i],
                                            context, "FieldNode");
        }

        // Java: for (MethodNode method : clazz.methods) methodList.add(performVote(methodTransformers, method, context))
        for (size_t i = 0; i < clazz->methodCount; i++)
        {
            size_t methodTransformerCount = 0;
            LIBMATTI_MLA_ITransformer **methodTransformers = LIBMATTI_ML_TransformStore_GetTransformersForMethod(
                transformer->transformers, className, clazz->methods[i], &methodTransformerCount);
            clazz->methods[i] = perform_vote(transformer, methodTransformers, methodTransformerCount, clazz->methods[i],
                                             context, "MethodNode");
        }

        // Java: clazz = performVote(classTransformers, clazz, context)
        classTransformers = LIBMATTI_ML_TransformStore_GetTransformersForClass(
            transformer->transformers, className, LIBMATTI_MLA_TargetType_Class(), &classTransformerCount);
        clazz = perform_vote(transformer, classTransformers, classTransformerCount, clazz, context, "ClassNode");

        LIBMATTI_ML_VotingContext_Free(context);
    }

    // Java: final int postFlags = pluginHandler.offerClassNodeToPlugins(Phase.AFTER, set.getOrDefault(AFTER,List.of()), clazz, classDesc, auditTrail, reason)
    int postFlags = LIBMATTI_ML_LaunchPluginHandler_OfferClassNodeToPlugins(
        transformer->pluginHandler, LIBMATTI_MLS_PHASE_AFTER, launchPluginTransformerSet.after,
        launchPluginTransformerSet.afterCount, clazz, classDesc, transformer->auditTrail, reason);

    if (preFlags == LIBMATTI_MLS_COMPUTE_NO_REWRITE && postFlags == LIBMATTI_MLS_COMPUTE_NO_REWRITE && !needsTransforming)
    {
        unsigned char *result = malloc(inputLength > 0 ? inputLength : 1);
        if (inputLength > 0) memcpy(result, inputClass, inputLength);
        *outLength = inputLength;
        LIBMATTI_ASMT_ClassNode_Free(clazz);
        free(launchPluginTransformerSet.before);
        free(launchPluginTransformerSet.after);
        free(internalName);
        LIBMATTI_ASM_Type_Free(classDesc);
        return result;
    }

    // Java: int mergedFlags = needsTransforming ? ComputeFlags.COMPUTE_FRAMES : (postFlags | preFlags)
    int mergedFlags = needsTransforming ? LIBMATTI_MLS_COMPUTE_FRAMES : (postFlags | preFlags);

    // Java: if (reason.equals(ITransformerActivity.COMPUTING_FRAMES_REASON)) mergedFlags &= ~COMPUTE_FRAMES
    if (strcmp(reason, LIBMATTI_MLA_ITransformerActivity_COMPUTING_FRAMES_REASON) == 0)
        mergedFlags &= ~LIBMATTI_MLS_COMPUTE_FRAMES;

    // Java: final ClassWriter cw = TransformerClassWriter.createClassWriter(mergedFlags, this, clazz); clazz.accept(cw)
    LIBMATTI_ASM_ClassWriter *writer = LIBMATTI_ML_TransformerClassWriter_CreateClassWriter(
        mergedFlags, transformer, clazz);
    LIBMATTI_ASM_ClassWriter_Accept(writer, clazz);

    unsigned char *result = LIBMATTI_ASM_ClassWriter_ToByteArray(writer, outLength);

    LIBMATTI_ASM_ClassWriter_Free(writer);
    LIBMATTI_ASMT_ClassNode_Free(clazz);
    free(launchPluginTransformerSet.before);
    free(launchPluginTransformerSet.after);
    free(internalName);
    LIBMATTI_ASM_Type_Free(classDesc);

    return result;
}

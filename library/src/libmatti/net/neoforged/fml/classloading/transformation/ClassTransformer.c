// Port of net.neoforged.fml.classloading.transformation.ClassTransformer.

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassTransformer.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/security/MessageDigest.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassTransformStatistics.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/TransformerClassWriter.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"
#include "libmatti/org/objectweb/asm/ClassReader.h"
#include "libmatti/org/objectweb/asm/ClassWriter.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"
#include "libmatti/org/objectweb/asm/Type.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static MessageDigest getSha256()
static LIBMATTI_JS_MessageDigest *get_sha256(void)
{
    return LIBMATTI_JS_MessageDigest_GetInstance("SHA-256");
}

// Java: private static final byte[] EMPTY = getSha256().digest(new byte[0])
static unsigned char *empty_digest;

// Java: Supplier<byte[]> digest
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

// Java: private static ClassWriter createClassWriter(ComputeFlags flags, ClassNode clazzAccessor, ClassHierarchyRecomputationContext locator)
static LIBMATTI_ASM_ClassWriter *create_class_writer(LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags flags,
                                                     LIBMATTI_ASMT_ClassNode *clazzAccessor)
{
    int writerFlag = 0;
    if (flags == LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_MAXS) writerFlag = LIBMATTI_ASM_COMPUTE_MAXS;
    else if (flags == LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES)
        writerFlag = LIBMATTI_ASM_COMPUTE_FRAMES;

    // Java: only the TransformerClassWriter when COMPUTE_FRAMES is set, as it's slower and only
    //       COMPUTE_FRAMES calls getCommonSuperClass
    return LIBMATTI_ASM_ClassWriter_New(writerFlag, clazzAccessor);
}

// Java: the audit trail callback handed to TransformationContext (BiConsumer<String, String[]>)
static void accept_audit(const char *activity, const char *const *context, size_t contextCount, void *userdata)
{
    LIBMATTI_FML_TransformerActivity_Accept((LIBMATTI_FML_TransformerActivity *) userdata, activity, context,
                                            contextCount);
}

LIBMATTI_FML_ClassTransformer *LIBMATTI_FML_ClassTransformer_New(LIBMATTI_FML_ClassProcessorSet *processors,
                                                               LIBMATTI_FML_ClassProcessorAuditLog *auditTrail)
{
    if (empty_digest == NULL)
    {
        LIBMATTI_JS_MessageDigest *digest = get_sha256();
        size_t emptyDigestLength = 0;
        empty_digest = LIBMATTI_JS_MessageDigest_Digest(digest, NULL, 0, &emptyDigestLength);
        LIBMATTI_JS_MessageDigest_Free(digest);
    }

    LIBMATTI_FML_ClassTransformer *transformer = calloc(1, sizeof(LIBMATTI_FML_ClassTransformer));
    transformer->processors = processors;
    transformer->auditTrail = auditTrail;
    return transformer;
}

void LIBMATTI_FML_ClassTransformer_Free(LIBMATTI_FML_ClassTransformer *transformer)
{
    free(transformer);
}

// Java: private static volatile Path tempDir - created lazily on the first dump
static char *dump_temp_dir = NULL;

// Java: private void dumpClass(byte[] clazz, String className)
static void dump_class(const unsigned char *clazz, size_t length, const char *className)
{
    // Java: if (tempDir == null) { synchronized { if (tempDir == null) tempDir = Files.createTempDirectory("classDump"); } }
    if (dump_temp_dir == NULL)
    {
        char *tempDir = LIBMATTI_JNF_Files_CreateTempDirectory("classDump");
        if (tempDir == NULL)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to create temporary directory");
            return;
        }
        dump_temp_dir = tempDir;
    }

    // Java: Path tempFile = tempDir.resolve(className + ".class"); Files.write(tempFile, clazz);
    size_t tempFileLength = strlen(dump_temp_dir) + 1 + strlen(className) + 6 + 1;
    char *tempFile = malloc(tempFileLength);
    snprintf(tempFile, tempFileLength, "%s/%s.class", dump_temp_dir, className);

    if (LIBMATTI_JNF_Files_Write(tempFile, clazz, length))
    {
        // Java: LOGGER.info("Wrote {} byte class file {} to {}", clazz.length, className, tempFile)
        char byteCount[32];
        snprintf(byteCount, sizeof(byteCount), "%zu", length);
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Wrote {} byte class file {} to {}", byteCount, className, tempFile);
    }
    else
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to write class file {}", className);
    }
    free(tempFile);
}

unsigned char *LIBMATTI_FML_ClassTransformer_Transform(LIBMATTI_FML_ClassTransformer *transformer,
                                                       const unsigned char *inputClass, size_t inputLength,
                                                       const char *className,
                                                       const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer,
                                                       LIBMATTI_FML_ClassHierarchyRecomputationContext *locator,
                                                       size_t *outLength)
{
    // Java: String internalName = className.replace('.', '/'); final Type classDesc = Type.getObjectType(internalName)
    char *internalName = strdup(className);
    for (char *c = internalName; *c != '\0'; c++) if (*c == '.') *c = '/';
    LIBMATTI_ASM_Type *classDesc = LIBMATTI_ASM_Type_GetObjectType(internalName);

    LIBMATTI_FML_ClassTransformStatistics_IncrementLoadedClasses();

    // Java: var transformersToUse = this.processors.transformersFor(classDesc, inputClass.length == 0, upToTransformer)
    size_t transformersToUseCount = 0;
    LIBMATTI_NEOFORGESPI_ClassProcessor **transformersToUse = LIBMATTI_FML_ClassProcessorSet_TransformersFor(
        transformer->processors, classDesc, inputLength == 0, upToTransformer, &transformersToUseCount);

    if (transformersToUseCount == 0)
    {
        // Java: return inputClass
        unsigned char *result = malloc(inputLength > 0 ? inputLength : 1);
        if (inputLength > 0) memcpy(result, inputClass, inputLength);
        *outLength = inputLength;
        free(transformersToUse);
        free(internalName);
        LIBMATTI_ASM_Type_Free(classDesc);
        return result;
    }

    LIBMATTI_FML_ClassTransformStatistics_IncrementTransformedClasses();

    // Java: Supplier<byte[]> digest; ClassNode clazz = new ClassNode(Opcodes.ASM9); boolean isEmpty = inputClass.length == 0;
    DigestSource digestSource = {inputClass, inputLength};
    LIBMATTI_ASMT_ClassNode *clazz = LIBMATTI_ASMT_ClassNode_New();
    int isEmpty = inputLength == 0;
    unsigned char *(*digest)(void *userdata, size_t *length) = digest_source;

    if (inputLength > 0)
    {
        // Java: new ClassReader(inputClass).accept(clazz, ClassReader.EXPAND_FRAMES)
        LIBMATTI_ASM_ClassReader *reader = LIBMATTI_ASM_ClassReader_New(inputClass, inputLength,
                                                                       LIBMATTI_ASM_EXPAND_FRAMES);
        LIBMATTI_ASM_ClassReader_Accept(reader, clazz);
        LIBMATTI_ASM_ClassReader_Free(reader);
    }
    else
    {
        // Java: clazz.name = classDesc.getInternalName(); clazz.version = Opcodes.V1_8; clazz.superName = "java/lang/Object"
        clazz->name = strdup(internalName);
        clazz->version = 52;
        clazz->superName = strdup("java/lang/Object");
    }

    int allowsComputeFrames = 0;
    LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags flags =
        LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;

    for (size_t i = 0; i < transformersToUseCount; i++)
    {
        LIBMATTI_NEOFORGESPI_ClassProcessor *processor = transformersToUse[i];
        LIBMATTI_NEOFORGESPI_ProcessorName *name = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(processor);

        // Java: if (ClassProcessorIds.COMPUTING_FRAMES.equals(transformer.name())) { allowsComputeFrames = true; continue; }
        if (LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(
                name, LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES()) == 0)
        {
            allowsComputeFrames = 1;
            continue;
        }

        // Java: var trail = auditTrail.forClassProcessor(classDesc.getClassName(), transformer)
        char *classDescName = LIBMATTI_ASM_Type_GetClassName(classDesc);
        LIBMATTI_FML_TransformerActivity *trail =
            LIBMATTI_FML_ClassProcessorAuditLog_ForClassProcessor(transformer->auditTrail, classDescName, processor);
        free(classDescName);

        // Java: var context = new ClassProcessor.TransformationContext(classDesc, clazz, isEmpty, trail, digest)
        LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context =
            LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_New(classDesc, clazz, isEmpty, accept_audit,
                                                                         trail, digest, &digestSource);

        LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags newFlags =
            LIBMATTI_NEOFORGESPI_ClassProcessor_ProcessClass(processor, context);
        LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Free(context);

        if (newFlags != LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE)
        {
            // Java: trail.rewrites(); isEmpty = false;
            LIBMATTI_FML_TransformerActivity_Rewrites(trail);
            isEmpty = 0;
        }
        flags = LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_Max(flags, newFlags);

        if (flags >= LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES)
        {
            // Java: throw new IllegalStateException when the transformer may not recompute frames
            char *nameString = LIBMATTI_NEOFORGESPI_ProcessorName_ToString(name);
            char *framesString = LIBMATTI_NEOFORGESPI_ProcessorName_ToString(
                LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES());
            if (!LIBMATTI_FML_ClassProcessorSet_CanRecomputeFrames(transformer->processors, name))
            {
                LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                         "Transformer {} requested COMPUTE_FRAMES but does not depend, directly or "
                                         "indirectly, on running after {}",
                                         nameString, framesString);
            }
            if (!allowsComputeFrames)
            {
                LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                         "Transformer {} requested COMPUTE_FRAMES but is not allowed to do so as it "
                                         "runs before transformer {}",
                                         nameString, framesString);
            }
            free(nameString);
            free(framesString);
        }
    }

    if (upToTransformer == NULL)
    {
        // Java: run post-result callbacks
        LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext context = {classDesc};
        for (size_t i = 0; i < transformersToUseCount; i++)
            LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessing(transformersToUse[i], &context);
    }

    if (flags == LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE)
    {
        // Java: return inputClass; // No changes were made
        unsigned char *result = malloc(inputLength > 0 ? inputLength : 1);
        if (inputLength > 0) memcpy(result, inputClass, inputLength);
        *outLength = inputLength;
        LIBMATTI_ASMT_ClassNode_Free(clazz);
        free(transformersToUse);
        free(internalName);
        LIBMATTI_ASM_Type_Free(classDesc);
        return result;
    }

    // Java: ClassWriter cw = createClassWriter(flags, clazz, locator); clazz.accept(cw); return cw.toByteArray()
    LIBMATTI_ASM_ClassWriter *writer = create_class_writer(flags, clazz);
    LIBMATTI_ASM_ClassWriter_Accept(writer, clazz);
    unsigned char *result = LIBMATTI_ASM_ClassWriter_ToByteArray(writer, outLength);

    // Java: if (LOGGER.isEnabled(Level.TRACE) && upToTransformer == null && LOGGER.isEnabled(Level.TRACE, CLASSDUMP)) dumpClass(cw.toByteArray(), className);
    static const LIBMATTI_ML_LogMarker CLASSDUMP_MARKER = {"CLASSDUMP"};
    if (upToTransformer == NULL && LIBMATTI_ML_Logger_IsEnabledTrace(LOGGER(), NULL) &&
        LIBMATTI_ML_Logger_IsEnabledTrace(LOGGER(), &CLASSDUMP_MARKER))
        dump_class(result, *outLength, className);

    LIBMATTI_ASM_ClassWriter_Free(writer);
    LIBMATTI_ASMT_ClassNode_Free(clazz);
    free(transformersToUse);
    free(internalName);
    LIBMATTI_ASM_Type_Free(classDesc);
    (void) locator;
    return result;
}

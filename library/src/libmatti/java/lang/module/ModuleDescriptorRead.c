// Port of java.lang.module.ModuleDescriptor.read (JDK: ModuleInfo.read).
// Parses a module-info.class directly: the constant pool, the access flags,
// and the Module/ModulePackages/ModuleMainClass/ModuleVersion attributes.

#include "libmatti/java/lang/module/ModuleDescriptorRead.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// little binary reader
// ---------------------------------------------------------------------------

typedef struct
{
    const unsigned char *data;
    size_t length;
    size_t position;
    int failed;
} ByteCursor;

static uint8_t u1(ByteCursor *c)
{
    if (c->position + 1 > c->length)
    {
        c->failed = 1;
        return 0;
    }
    return c->data[c->position++];
}

static uint16_t u2(ByteCursor *c)
{
    if (c->position + 2 > c->length)
    {
        c->failed = 1;
        return 0;
    }
    uint16_t value = (uint16_t) ((c->data[c->position] << 8) | c->data[c->position + 1]);
    c->position += 2;
    return value;
}

static uint32_t u4(ByteCursor *c)
{
    if (c->position + 4 > c->length)
    {
        c->failed = 1;
        return 0;
    }
    uint32_t value = ((uint32_t) c->data[c->position] << 24) | ((uint32_t) c->data[c->position + 1] << 16) |
                     ((uint32_t) c->data[c->position + 2] << 8) | c->data[c->position + 3];
    c->position += 4;
    return value;
}

static const unsigned char *bytes(ByteCursor *c, size_t length)
{
    if (c->position + length > c->length)
    {
        c->failed = 1;
        return NULL;
    }
    const unsigned char *data = c->data + c->position;
    c->position += length;
    return data;
}

// ---------------------------------------------------------------------------
// constant pool: only the Utf8 entries matter for the descriptor
// ---------------------------------------------------------------------------

typedef struct
{
    char **utf8;    // 1-based, index 0 unused; NULL for non-Utf8 entries
    size_t count;
} ConstantPool;

static void constant_pool_free(ConstantPool *pool)
{
    for (size_t i = 0; i < pool->count; i++)
        free(pool->utf8[i]);
    free(pool->utf8);
}

// Java: the constant pool parse; skips everything but CONSTANT_Utf8 (1)
static int constant_pool_read(ByteCursor *c, ConstantPool *pool)
{
    uint16_t count = u2(c);
    if (c->failed)
        return 0;

    pool->count = count;
    pool->utf8 = calloc(count, sizeof(char *)); // slot 0 unused

    for (uint16_t index = 1; index < count; index++)
    {
        uint8_t tag = u1(c);
        if (c->failed)
            return 0;

        switch (tag)
        {
            case 1: // CONSTANT_Utf8
            {
                uint16_t length = u2(c);
                const unsigned char *text = bytes(c, length);
                if (c->failed)
                    return 0;
                char *value = malloc((size_t) length + 1);
                memcpy(value, text, length);
                value[length] = '\0';
                pool->utf8[index] = value;
                break;
            }
            case 3: // CONSTANT_Integer
            case 4: // CONSTANT_Float
                c->position += 4;
                break;
            case 5: // CONSTANT_Long
            case 6: // CONSTANT_Double
                c->position += 8;
                index++; // Java: longs/doubles take two slots
                break;
            case 7: // CONSTANT_Class
            case 8: // CONSTANT_String
            case 16: // CONSTANT_MethodType
            case 19: // CONSTANT_Module
            case 20: // CONSTANT_Package
                c->position += 2;
                break;
            case 9: // CONSTANT_Fieldref
            case 10: // CONSTANT_Methodref
            case 11: // CONSTANT_InterfaceMethodref
            case 12: // CONSTANT_NameAndType
            case 17: // CONSTANT_Dynamic
            case 18: // CONSTANT_InvokeDynamic
                c->position += 4;
                break;
            case 15: // CONSTANT_MethodHandle
                c->position += 3;
                break;
            default:
                return 0; // Java: InvalidModuleDescriptorException
        }
    }
    return 1;
}

static const char *utf8(const ConstantPool *pool, uint16_t index)
{
    return index < pool->count ? pool->utf8[index] : NULL;
}

// ---------------------------------------------------------------------------
// attribute flags (JVM: ACC_ constants of the module attribute)
// ---------------------------------------------------------------------------

#define MODULE_ACC_OPEN 0x0020
#define REQUIRES_ACC_TRANSITIVE 0x0020
#define REQUIRES_ACC_STATIC_PHASE 0x0040
#define REQUIRES_ACC_SYNTHETIC 0x1000
#define REQUIRES_ACC_MANDATED 0x8000
#define EXPORTS_ACC_SYNTHETIC 0x1000
#define OPENS_ACC_SYNTHETIC 0x1000
#define PROVIDES_ACC_SYNTHETIC 0x1000

// Java: ModuleInfo.read - the attribute walk
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleDescriptor_ReadWithFlags(const unsigned char *bytes_, size_t length,
                                                                         int strict)
{
    (void) strict;
    ByteCursor c = {bytes_, length, 0, 0};

    // Java: magic 0xCAFEBABE, minor, major
    if (u4(&c) != 0xCAFEBABEU)
        return NULL;
    u2(&c); // minor
    u2(&c); // major
    if (c.failed)
        return NULL;

    ConstantPool pool;
    if (!constant_pool_read(&c, &pool))
    {
        constant_pool_free(&pool);
        return NULL;
    }

    uint16_t accessFlags = u2(&c);
    uint16_t thisClass = u2(&c);
    u2(&c); // superCount
    uint16_t interfaceCount = u2(&c);
    c.position += (size_t) interfaceCount * 2;
    uint16_t fieldCount = u2(&c);
    c.position += (size_t) fieldCount * 6; // each field: 6 bytes of header (attrs not present in module-info)
    uint16_t methodCount = u2(&c);
    c.position += (size_t) methodCount * 6;
    if (c.failed)
    {
        constant_pool_free(&pool);
        return NULL;
    }

    LIBMATTI_JL_ModuleDescriptor *descriptor = NULL;
    const char *moduleName = NULL;

    // Java: the attribute table walk; Module must be present
    uint16_t attributeCount = u2(&c);
    for (uint16_t i = 0; i < attributeCount && !c.failed; i++)
    {
        uint16_t nameIndex = u2(&c);
        uint32_t attributeLength = u4(&c);
        const char *attributeName = utf8(&pool, nameIndex);
        if (attributeName == NULL)
            break;

        size_t attributeEnd = c.position + attributeLength;

        if (strcmp(attributeName, "Module") == 0)
        {
            // Java: module_name_index, module_flags, module_version_index
            uint16_t nameIdx = u2(&c);
            uint16_t moduleFlags = u2(&c);
            u2(&c); // module_version_index
            moduleName = utf8(&pool, nameIdx);
            if (moduleName == NULL)
                break;

            descriptor = LIBMATTI_JL_ModuleDescriptor_Create(moduleName, NULL, 0);
            if (moduleFlags & MODULE_ACC_OPEN)
                LIBMATTI_JL_ModuleDescriptor_AddModifier(descriptor, LIBMATTI_JL_MODIFIER_OPEN);

            // Java: the requires table
            uint16_t requiresCount = u2(&c);
            for (uint16_t r = 0; r < requiresCount && !c.failed; r++)
            {
                uint16_t requireIdx = u2(&c);
                uint16_t requireFlags = u2(&c);
                uint16_t requireVersionIdx = u2(&c);
                const char *requireName = utf8(&pool, requireIdx);
                const char *requireVersion = utf8(&pool, requireVersionIdx);
                if (requireName != NULL)
                {
                    int modifiers = 0;
                    if (requireFlags & REQUIRES_ACC_TRANSITIVE)
                        modifiers |= LIBMATTI_JL_REQUIRES_MODIFIER_TRANSITIVE;
                    if (requireFlags & REQUIRES_ACC_STATIC_PHASE)
                        modifiers |= LIBMATTI_JL_REQUIRES_MODIFIER_STATIC_PHASE;
                    if (requireFlags & REQUIRES_ACC_SYNTHETIC)
                        modifiers |= LIBMATTI_JL_REQUIRES_MODIFIER_SYNTHETIC;
                    if (requireFlags & REQUIRES_ACC_MANDATED)
                        modifiers |= LIBMATTI_JL_REQUIRES_MODIFIER_MANDATED;
                    LIBMATTI_JL_ModuleDescriptor_AddRequires(descriptor, requireName, modifiers,
                                                             requireVersion != NULL ? requireVersion : NULL);
                }
            }

            // Java: the exports table
            uint16_t exportsCount = u2(&c);
            for (uint16_t e = 0; e < exportsCount && !c.failed; e++)
            {
                uint16_t exportIdx = u2(&c);
                uint16_t exportFlags = u2(&c);
                uint16_t exportToCount = u2(&c);
                char **targets = NULL;
                size_t targetCount = 0;
                for (uint16_t t = 0; t < exportToCount && !c.failed; t++)
                {
                    uint16_t targetIdx = u2(&c);
                    const char *target = utf8(&pool, targetIdx);
                    if (target != NULL)
                    {
                        targets = realloc(targets, sizeof(char *) * (targetCount + 1));
                        targets[targetCount++] = strdup(target);
                    }
                }
                const char *source = utf8(&pool, exportIdx);
                if (source != NULL)
                    LIBMATTI_JL_ModuleDescriptor_AddExports(descriptor, source, targets, targetCount);
                for (size_t t = 0; t < targetCount; t++)
                    free(targets[t]);
                free(targets);
                (void) exportFlags;
            }

            // Java: the opens table
            uint16_t opensCount = u2(&c);
            for (uint16_t o = 0; o < opensCount && !c.failed; o++)
            {
                uint16_t openIdx = u2(&c);
                uint16_t openFlags = u2(&c);
                uint16_t openToCount = u2(&c);
                char **targets = NULL;
                size_t targetCount = 0;
                for (uint16_t t = 0; t < openToCount && !c.failed; t++)
                {
                    uint16_t targetIdx = u2(&c);
                    const char *target = utf8(&pool, targetIdx);
                    if (target != NULL)
                    {
                        targets = realloc(targets, sizeof(char *) * (targetCount + 1));
                        targets[targetCount++] = strdup(target);
                    }
                }
                const char *source = utf8(&pool, openIdx);
                if (source != NULL)
                    LIBMATTI_JL_ModuleDescriptor_AddOpens(descriptor, source, targets, targetCount);
                for (size_t t = 0; t < targetCount; t++)
                    free(targets[t]);
                free(targets);
                (void) openFlags;
            }

            // Java: the uses table
            uint16_t usesCount = u2(&c);
            for (uint16_t u = 0; u < usesCount && !c.failed; u++)
            {
                uint16_t useIdx = u2(&c);
                const char *use = utf8(&pool, useIdx);
                if (use != NULL)
                    LIBMATTI_JL_ModuleDescriptor_AddUses(descriptor, use);
            }

            // Java: the provides table
            uint16_t providesCount = u2(&c);
            for (uint16_t p = 0; p < providesCount && !c.failed; p++)
            {
                uint16_t provideIdx = u2(&c);
                uint16_t provideWithCount = u2(&c);
                char **providers = NULL;
                size_t providerCount = 0;
                for (uint16_t w = 0; w < provideWithCount && !c.failed; w++)
                {
                    uint16_t withIdx = u2(&c);
                    const char *with = utf8(&pool, withIdx);
                    if (with != NULL)
                    {
                        providers = realloc(providers, sizeof(char *) * (providerCount + 1));
                        providers[providerCount++] = strdup(with);
                    }
                }
                const char *service = utf8(&pool, provideIdx);
                if (service != NULL)
                    LIBMATTI_JL_ModuleDescriptor_AddProvides(descriptor, service, providers, providerCount);
                for (size_t w = 0; w < providerCount; w++)
                    free(providers[w]);
                free(providers);
            }
        }
        else if (strcmp(attributeName, "ModulePackages") == 0 && descriptor != NULL)
        {
            // Java: package_count + package_index[]
            uint16_t packageCount = u2(&c);
            for (uint16_t p = 0; p < packageCount && !c.failed; p++)
            {
                uint16_t packageIdx = u2(&c);
                const char *package = utf8(&pool, packageIdx);
                if (package != NULL)
                    LIBMATTI_JL_ModuleDescriptor_AddPackage(descriptor, package);
            }
        }
        else if (strcmp(attributeName, "ModuleMainClass") == 0 && descriptor != NULL)
        {
            uint16_t mainIdx = u2(&c);
            const char *mainClass = utf8(&pool, mainIdx);
            if (mainClass != NULL)
                LIBMATTI_JL_ModuleDescriptor_SetMainClass(descriptor, mainClass);
        }
        else if (strcmp(attributeName, "ModuleVersion") == 0 && descriptor != NULL)
        {
            uint16_t versionIdx = u2(&c);
            const char *version = utf8(&pool, versionIdx);
            if (version != NULL)
                LIBMATTI_JL_ModuleDescriptor_SetVersion(descriptor, version);
        }

        // Java: the attribute walk continues at the declared length
        c.position = attributeEnd;
    }

    constant_pool_free(&pool);
    return descriptor;
}

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleDescriptor_Read(const unsigned char *bytes, size_t length)
{
    return LIBMATTI_JL_ModuleDescriptor_ReadWithFlags(bytes, length, 0);
}

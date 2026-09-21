#include "Configuration.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JL_ResolvedModule **LIBMATTI_JL_Configuration_Modules(const LIBMATTI_JL_Configuration *configuration, size_t *count)
{
    *count = configuration->moduleCount;
    return configuration->modules;
}

LIBMATTI_JL_ResolvedModule *LIBMATTI_JL_Configuration_FindModule(const LIBMATTI_JL_Configuration *configuration, const char *name)
{
    // Java: Optional<ResolvedModule> findModule(String name)
    for (size_t i = 0; i < configuration->moduleCount; i++)
    {
        if (strcmp(configuration->modules[i]->name, name) == 0) return configuration->modules[i];
    }
    return NULL;
}

static int nameInList(char **names, size_t count, const char *name)
{
    for (size_t i = 0; i < count; i++)
    {
        if (strcmp(names[i], name) == 0) return 1;
    }
    return 0;
}

// Java: Configuration.resolve + resolveAndBind share the Resolver; bindService (the
// 'bind' flag) additionally resolves providers of every used service into the graph.
static LIBMATTI_JL_Configuration *resolve_internal(LIBMATTI_JL_ModuleReference **references, size_t referenceCount,
                                                   const char **roots, size_t rootCount, int bindService)
{
    // Java (Resolver): DFS from the roots; each module's non-static requires
    // (Requires.Modifier.STATIC is compile-time only) are resolved transitively.

    // name -> reference lookup over the 'before' finder's modules
    // (Java builds the same lookup while searching 'before' then 'after')
    char **resolvedNames = NULL;
    size_t resolvedCount = 0;
    LIBMATTI_JL_ModuleReference **resolvedRefs = NULL;

    // DFS queue (Java: Deque<ModuleDescriptor>)
    char **queue = NULL;
    size_t queueHead = 0, queueTail = 0;
    for (size_t i = 0; i < rootCount; i++)
    {
        queue = realloc(queue, sizeof(char *) * (queueTail + 1));
        queue[queueTail++] = (char *)roots[i]; // borrowed, never freed
    }

    while (queueHead < queueTail)
    {
        char *name = queue[queueHead++];
        if (nameInList(resolvedNames, resolvedCount, name)) continue;

        // Java: finder.find(name).orElseThrow(FindException) - 'before' finder only here
        LIBMATTI_JL_ModuleReference *ref = NULL;
        for (size_t i = 0; i < referenceCount; i++)
        {
            if (strcmp(LIBMATTI_JL_ModuleDescriptor_Name(LIBMATTI_JL_ModuleReference_Descriptor(references[i])), name) == 0)
            {
                ref = references[i];
                break;
            }
        }
        if (ref == NULL)
        {
            // Java: FindException("Module " + name + " not found")
            for (size_t i = 0; i < resolvedCount; i++) free(resolvedNames[i]);
            free(resolvedNames);
            free(resolvedRefs);
            free(queue);
            return NULL;
        }

        resolvedNames = realloc(resolvedNames, sizeof(char *) * (resolvedCount + 1));
        resolvedRefs = realloc(resolvedRefs, sizeof(LIBMATTI_JL_ModuleReference *) * (resolvedCount + 1));
        resolvedNames[resolvedCount] = strdup(name);
        resolvedRefs[resolvedCount] = ref;
        resolvedCount++;

        // process dependences: only required at compile time (STATIC) is skipped
        size_t reqCount = 0;
        LIBMATTI_JL_ModuleDescriptor_Requires **requires = LIBMATTI_JL_ModuleDescriptor_GetRequires(LIBMATTI_JL_ModuleReference_Descriptor(ref), &reqCount);
        for (size_t r = 0; r < reqCount; r++)
        {
            if (requires[r]->modifiers & LIBMATTI_JL_REQUIRES_MODIFIER_STATIC_PHASE) continue;
            char *dn = requires[r]->name;
            if (!nameInList(resolvedNames, resolvedCount, dn) && !nameInList(queue + queueHead, queueTail - queueHead, dn))
            {
                queue = realloc(queue, sizeof(char *) * (queueTail + 1));
                queue[queueTail++] = dn; // borrowed, never freed
            }
        }
        // Java (Resolver.bind): a module that 'uses' a service pulls every module
        // that 'provides' it into the resolution graph, transitively.
        if (bindService)
        {
            size_t usesCount = 0;
            char **uses = LIBMATTI_JL_ModuleDescriptor_Uses(LIBMATTI_JL_ModuleReference_Descriptor(ref), &usesCount);
            for (size_t u = 0; u < usesCount; u++)
            {
                for (size_t p = 0; p < referenceCount; p++)
                {
                    LIBMATTI_JL_ModuleDescriptor *pdesc = LIBMATTI_JL_ModuleReference_Descriptor(references[p]);
                    size_t providesCount = 0;
                    LIBMATTI_JL_ModuleDescriptor_Provides **provides =
                        LIBMATTI_JL_ModuleDescriptor_GetProvides(pdesc, &providesCount);
                    for (size_t pr = 0; pr < providesCount; pr++)
                    {
                        if (strcmp(provides[pr]->service, uses[u]) != 0) continue;
                        const char *providerName = LIBMATTI_JL_ModuleDescriptor_Name(pdesc);
                        if (!nameInList(resolvedNames, resolvedCount, providerName) &&
                            !nameInList(queue + queueHead, queueTail - queueHead, providerName))
                        {
                            queue = realloc(queue, sizeof(char *) * (queueTail + 1));
                            queue[queueTail++] = (char *) providerName; // borrowed
                        }
                    }
                }
            }
        }
    }
    free(queue);

    LIBMATTI_JL_Configuration *configuration = calloc(1, sizeof(LIBMATTI_JL_Configuration));
    configuration->modules = calloc(resolvedCount, sizeof(LIBMATTI_JL_ResolvedModule *));
    configuration->moduleCount = resolvedCount;
    for (size_t i = 0; i < resolvedCount; i++)
    {
        LIBMATTI_JL_ResolvedModule *module = calloc(1, sizeof(LIBMATTI_JL_ResolvedModule));
        module->configuration = configuration;
        module->name = resolvedNames[i];
        module->reference = resolvedRefs[i];
        configuration->modules[i] = module;
    }
    free(resolvedNames);
    free(resolvedRefs);

    // Java (Configuration.build): reads edges - automatic modules read all other
    // modules in the configuration; otherwise every requires edge whose target is
    // present in the configuration.
    for (size_t i = 0; i < resolvedCount; i++)
    {
        LIBMATTI_JL_ResolvedModule *module = configuration->modules[i];
        const LIBMATTI_JL_ModuleDescriptor *desc = LIBMATTI_JL_ModuleReference_Descriptor(module->reference);
        if (LIBMATTI_JL_ModuleDescriptor_IsAutomatic(desc))
        {
            for (size_t j = 0; j < resolvedCount; j++)
            {
                if (j == i) continue;
                module->reads = realloc(module->reads, sizeof(LIBMATTI_JL_ResolvedModule *) * (module->readCount + 1));
                module->reads[module->readCount++] = configuration->modules[j];
            }
        }
        else
        {
            size_t reqCount = 0;
            LIBMATTI_JL_ModuleDescriptor_Requires **requires = LIBMATTI_JL_ModuleDescriptor_GetRequires((LIBMATTI_JL_ModuleDescriptor *)desc, &reqCount);
            for (size_t r = 0; r < reqCount; r++)
            {
                LIBMATTI_JL_ResolvedModule *other = LIBMATTI_JL_Configuration_FindModule(configuration, requires[r]->name);
                if (other != NULL)
                {
                    module->reads = realloc(module->reads, sizeof(LIBMATTI_JL_ResolvedModule *) * (module->readCount + 1));
                    module->reads[module->readCount++] = other;
                }
            }
        }
    }

    // Java (Resolver.bind / Configuration.build): a module that 'uses' a service also
    // reads every resolved module that 'provides' the service.
    for (size_t i = 0; i < configuration->moduleCount; i++)
    {
        LIBMATTI_JL_ResolvedModule *module = configuration->modules[i];
        size_t usesCount = 0;
        char **uses = LIBMATTI_JL_ModuleDescriptor_Uses(LIBMATTI_JL_ModuleReference_Descriptor(module->reference),
                                                        &usesCount);
        for (size_t u = 0; u < usesCount; u++)
        {
            for (size_t j = 0; j < configuration->moduleCount; j++)
            {
                LIBMATTI_JL_ResolvedModule *other = configuration->modules[j];
                size_t providesCount = 0;
                LIBMATTI_JL_ModuleDescriptor_Provides **provides = LIBMATTI_JL_ModuleDescriptor_GetProvides(
                    LIBMATTI_JL_ModuleReference_Descriptor(other->reference), &providesCount);
                int providesService = 0;
                for (size_t pr = 0; pr < providesCount && !providesService; pr++)
                    providesService = strcmp(provides[pr]->service, uses[u]) == 0;
                if (!providesService) continue;

                int alreadyReads = 0;
                for (size_t r = 0; r < module->readCount && !alreadyReads; r++)
                    alreadyReads = module->reads[r] == other;
                if (alreadyReads) continue;

                module->reads = realloc(module->reads,
                                        sizeof(LIBMATTI_JL_ResolvedModule *) * (module->readCount + 1));
                module->reads[module->readCount++] = other;
            }
        }
    }

    return configuration;
}

// Java: static Configuration resolve(ModuleFinder before, ModuleFinder after, Collection<String> roots)
LIBMATTI_JL_Configuration *LIBMATTI_JL_Configuration_Resolve(LIBMATTI_JL_ModuleReference **references, size_t referenceCount,
                                                             const char **roots, size_t rootCount)
{
    return resolve_internal(references, referenceCount, roots, rootCount, 0);
}

// Java: static Configuration resolveAndBind(ModuleFinder before, ModuleFinder after, Collection<String> roots)
LIBMATTI_JL_Configuration *LIBMATTI_JL_Configuration_ResolveAndBind(LIBMATTI_JL_ModuleReference **references,
                                                                     size_t referenceCount,
                                                                     const char **roots, size_t rootCount)
{
    return resolve_internal(references, referenceCount, roots, rootCount, 1);
}

void LIBMATTI_JL_Configuration_Free(LIBMATTI_JL_Configuration *configuration)
{
    for (size_t i = 0; i < configuration->moduleCount; i++)
    {
        free(configuration->modules[i]->name);
        free(configuration->modules[i]->reads);
        free(configuration->modules[i]);
    }
    free(configuration->modules);
    free(configuration);
}
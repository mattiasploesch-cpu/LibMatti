













#ifndef _GNU_SOURCE
#define _GNU_SOURCE

#endif

#include "libmatti/matti/mixin/MixinHookTable.h"

#if defined(__APPLE__)

// macOS has no ELF loader and the release does not build the mod targets.
// Keep the API linkable for the shared library, but no mixins are registered.
LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_New(void)
{
    return NULL;
}

LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_Default(void)
{
    return NULL;
}

size_t LIBMATTI_MIXIN_HookTable_AddObject(LIBMATTI_MIXIN_HookTable *table, const char *path,
                                          const char *modId)
{
    (void) table;
    (void) path;
    (void) modId;
    return 0;
}

void LIBMATTI_MIXIN_HookTable_RegisterTarget(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                             void *original)
{
    (void) table;
    (void) target;
    (void) original;
}

void LIBMATTI_MIXIN_HookTable_AddHook(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                      MattiMixinHandler handler, void *userdata, MattiMixinAt at,
                                      int priority, const char *modId)
{
    (void) table;
    (void) target;
    (void) handler;
    (void) userdata;
    (void) at;
    (void) priority;
    (void) modId;
}

const LIBMATTI_MIXIN_Target *LIBMATTI_MIXIN_Lookup(const LIBMATTI_MIXIN_HookTable *table,
                                                   const char *target)
{
    (void) table;
    (void) target;
    return NULL;
}

int LIBMATTI_MIXIN_Invoke(const LIBMATTI_MIXIN_HookTable *table, const char *target, void *owner,
                          void **args, size_t argCount, MattiMixinResult *result)
{
    (void) table;
    (void) target;
    (void) owner;
    (void) args;
    (void) argCount;
    if (result != NULL) *result = MATTI_MIXIN_PASS;
    return 1;
}

size_t LIBMATTI_MIXIN_GotPatch(const char *path, const char *const *symbolNames, size_t symbolCount,
                               MattiMixinHandler hook, void *userdata, const char *modId)
{
    (void) path;
    (void) symbolNames;
    (void) symbolCount;
    (void) hook;
    (void) userdata;
    (void) modId;
    return 0;
}

void *LIBMATTI_MIXIN_GotOriginal(const char *path, const char *symbolName)
{
    (void) path;
    (void) symbolName;
    return NULL;
}

void LIBMATTI_MIXIN_HookTable_Free(LIBMATTI_MIXIN_HookTable *table)
{
    (void) table;
}

#else

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/Logging.h"

#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}



static const char MIXIN_SECTION[] = MATTI_MIXIN_SECTION;









static const char *object_file_path(const char *path)
{
    if (path == NULL || path[0] == '\0') return "/proc/self/exe";
    return path;
}





static void *object_handle(const char *path)
{
    if (path == NULL || strcmp(path, "/proc/self/exe") == 0) return dlopen(NULL, RTLD_LAZY);

    void *handle = dlopen(path, RTLD_LAZY | RTLD_NOLOAD);
    if (handle != NULL) return handle;

    handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (handle == NULL)
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_CORE,
                                 "Mixin setup: {} is not a loadable shared object", path);
        return NULL;
    }
    return handle;
}

// The load bias of the object the handle belongs to (Java: none - the JVM relocates in memory).
// Bias 0 is valid (a non-PIE executable); only a missing link map is a failure.
#if defined(__ANDROID__)
typedef struct ObjectBiasSearch
{
    const char *path;
    uintptr_t bias;
    int found;
} ObjectBiasSearch;

static int find_object_bias(struct dl_phdr_info *info, size_t size, void *data)
{
    (void) size;
    ObjectBiasSearch *search = data;
    const char *name = info->dlpi_name != NULL ? info->dlpi_name : "";
    if (search->path == NULL || strcmp(search->path, "/proc/self/exe") == 0)
    {
        if (name[0] != '\0') return 0;
    }
    else if (strcmp(name, search->path) != 0)
        return 0;

    search->bias = (uintptr_t) info->dlpi_addr;
    search->found = 1;
    return 1;
}
#endif

static int object_load_bias(void *handle, const char *path, unsigned char **outBias)
{
#if defined(__ANDROID__)
    (void) handle;
    ObjectBiasSearch search = {path, 0, 0};
    if (!dl_iterate_phdr(find_object_bias, &search) || !search.found) return 0;
    *outBias = (unsigned char *) search.bias;
    return 1;
#else
    struct link_map *map = NULL;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) != 0 || map == NULL) return 0;
    *outBias = (unsigned char *) map->l_addr;
    return 1;
#endif
}





static int read_file_section(const char *filePath, const char *sectionName, Elf64_Addr *outAddr,
                             Elf64_Xword *outSize, Elf64_Word *outType)
{
    FILE *file = fopen(filePath, "rb");
    if (file == NULL) return 0;

    Elf64_Ehdr ehdr;
    int found = 0;
    if (fread(&ehdr, sizeof(ehdr), 1, file) == 1 && ehdr.e_ident[4] == ELFCLASS64 &&
        ehdr.e_shoff != 0 && ehdr.e_shnum != 0 && ehdr.e_shstrndx < ehdr.e_shnum)
    {
        Elf64_Shdr *sections = calloc(ehdr.e_shnum, sizeof(Elf64_Shdr));
        if (sections != NULL &&
            fseek(file, (long) ehdr.e_shoff, SEEK_SET) == 0 &&
            fread(sections, sizeof(Elf64_Shdr), ehdr.e_shnum, file) == ehdr.e_shnum)
        {
            Elf64_Shdr *nameHeader = &sections[ehdr.e_shstrndx];
            char *nameTable = nameHeader->sh_size != 0 ? malloc((size_t) nameHeader->sh_size + 1) : NULL;
            if (nameTable != NULL &&
                fseek(file, (long) nameHeader->sh_offset, SEEK_SET) == 0 &&
                fread(nameTable, 1, (size_t) nameHeader->sh_size, file) == nameHeader->sh_size)
            {
                nameTable[nameHeader->sh_size] = '\0';
                for (uint16_t i = 0; i < ehdr.e_shnum; i++)
                {
                    if (strcmp(nameTable + sections[i].sh_name, sectionName) != 0) continue;

                    *outAddr = sections[i].sh_addr;
                    *outSize = sections[i].sh_size;
                    if (outType != NULL) *outType = sections[i].sh_type;
                    found = 1;
                    break;
                }
            }
            free(nameTable);
        }
        free(sections);
    }
    fclose(file);
    return found;
}






struct LIBMATTI_MIXIN_HookTable
{
    LIBMATTI_MIXIN_Target *targets;
    size_t targetCount;
};

// Java: Arrays.binarySearch-style ordering by (priority, registration order)
static void sort_hooks(LIBMATTI_MIXIN_Hook *hooks, size_t count)
{
    for (size_t i = 1; i < count; i++)
    {
        LIBMATTI_MIXIN_Hook current = hooks[i];
        size_t j = i;
        while (j > 0 && hooks[j - 1].priority > current.priority)
        {
            hooks[j] = hooks[j - 1];
            j--;
        }
        hooks[j] = current;
    }
}

static LIBMATTI_MIXIN_Target *find_target(LIBMATTI_MIXIN_HookTable *table, const char *target)
{
    for (size_t i = 0; i < table->targetCount; i++)
        if (strcmp(table->targets[i].target, target) == 0) return &table->targets[i];

    return NULL;
}

LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_New(void)
{
    return calloc(1, sizeof(LIBMATTI_MIXIN_HookTable));
}

// Java: MixinEnvironment holds the process-wide setup.
static LIBMATTI_MIXIN_HookTable *defaultTable = NULL;

LIBMATTI_MIXIN_HookTable *LIBMATTI_MIXIN_HookTable_Default(void)
{
    if (defaultTable == NULL) defaultTable = LIBMATTI_MIXIN_HookTable_New();
    return defaultTable;
}





static int entry_is_target_marker(const MattiMixinEntry *entry)
{
    return entry->modId == NULL && entry->userdata == NULL && entry->at == MATTI_MIXIN_AT_HEAD &&
           entry->priority == 0;
}

size_t LIBMATTI_MIXIN_HookTable_AddObject(LIBMATTI_MIXIN_HookTable *table, const char *path,
                                          const char *modId)
{
    if (table == NULL) return 0;

    void *handle = object_handle(path);
    if (handle == NULL) return 0;
    unsigned char *base = NULL;
    if (!object_load_bias(handle, path, &base))
    {
        dlclose(handle);
        return 0;
    }

    Elf64_Addr sectionAddress = 0;
    Elf64_Xword sectionSize = 0;
    if (!read_file_section(object_file_path(path), MIXIN_SECTION, &sectionAddress, &sectionSize, NULL) ||
        sectionAddress == 0 || sectionSize < sizeof(MattiMixinEntry))
    {
        dlclose(handle);
        return 0;
    }




    const MattiMixinEntry *entries = (const MattiMixinEntry *) (base + sectionAddress);
    size_t taken = 0;
    for (size_t i = 0; i < sectionSize / sizeof(MattiMixinEntry); i++)
    {
        if (entries[i].handler == NULL) continue;

        if (entry_is_target_marker(&entries[i]))
        {
            LIBMATTI_MIXIN_HookTable_RegisterTarget(table, entries[i].target,
                                                    (void *) entries[i].handler);
            taken++;
            continue;
        }

        LIBMATTI_MIXIN_HookTable_AddHook(table, entries[i].target, entries[i].handler,
                                         entries[i].userdata, (MattiMixinAt) entries[i].at,
                                         entries[i].priority,
                                         entries[i].modId != NULL ? entries[i].modId : modId);
        taken++;
    }





    if (taken == 0) dlclose(handle);
    return taken;
}

void LIBMATTI_MIXIN_HookTable_RegisterTarget(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                             void *original)
{
    if (table == NULL || target == NULL) return;

    LIBMATTI_MIXIN_Target *existing = find_target(table, target);
    if (existing != NULL)
    {
        existing->original = original;
        return;
    }

    table->targets = realloc(table->targets, sizeof(*table->targets) * (table->targetCount + 1));
    LIBMATTI_MIXIN_Target *entry = &table->targets[table->targetCount++];
    entry->target = strdup(target);
    entry->original = original;
    entry->hooks = NULL;
    entry->hookCount = 0;
}

void LIBMATTI_MIXIN_HookTable_AddHook(LIBMATTI_MIXIN_HookTable *table, const char *target,
                                      MattiMixinHandler handler, void *userdata, MattiMixinAt at,
                                      int priority, const char *modId)
{
    if (table == NULL || target == NULL || handler == NULL) return;

    LIBMATTI_MIXIN_Target *entry = find_target(table, target);
    if (entry == NULL)
    {
        LIBMATTI_MIXIN_HookTable_RegisterTarget(table, target, NULL);
        entry = find_target(table, target);
    }

    entry->hooks = realloc(entry->hooks, sizeof(*entry->hooks) * (entry->hookCount + 1));
    LIBMATTI_MIXIN_Hook *hook = &entry->hooks[entry->hookCount++];
    hook->handler = handler;
    hook->userdata = userdata;
    hook->at = at;
    hook->priority = priority;
    hook->modId = modId != NULL ? strdup(modId) : NULL;

    sort_hooks(entry->hooks, entry->hookCount);
}

const LIBMATTI_MIXIN_Target *LIBMATTI_MIXIN_Lookup(const LIBMATTI_MIXIN_HookTable *table,
                                                   const char *target)
{
    if (table == NULL || target == NULL) return NULL;
    return find_target((LIBMATTI_MIXIN_HookTable *) table, target);
}

int LIBMATTI_MIXIN_Invoke(const LIBMATTI_MIXIN_HookTable *table, const char *target, void *owner,
                          void **args, size_t argCount, MattiMixinResult *result)
{
    const LIBMATTI_MIXIN_Target *entry = LIBMATTI_MIXIN_Lookup(table, target);
    if (result != NULL) *result = MATTI_MIXIN_PASS;
    if (entry == NULL || entry->hookCount == 0) return 1;

    MattiMixinCallbackInfo info = {target, owner, args, argCount, MATTI_MIXIN_PASS};
    for (size_t i = 0; i < entry->hookCount; i++)
    {
        info.result = MATTI_MIXIN_PASS;
        entry->hooks[i].handler(&info, entry->hooks[i].userdata);

        if (info.result != MATTI_MIXIN_CANCEL) continue;

        if (result != NULL) *result = MATTI_MIXIN_CANCEL;

// Java: a cancellation at HEAD skips the target and the rest of the chain; a cancellation
        // after the target ran (RETURN/TAIL) only stops the remaining handlers.
        return entry->hooks[i].at == MATTI_MIXIN_AT_HEAD ? 0 : 1;
    }

    return 1;
}

void LIBMATTI_MIXIN_HookTable_Free(LIBMATTI_MIXIN_HookTable *table)
{
    if (table == NULL) return;

    for (size_t i = 0; i < table->targetCount; i++)
    {
        free((void *) table->targets[i].target);
        for (size_t k = 0; k < table->targets[i].hookCount; k++)
            free((void *) table->targets[i].hooks[k].modId);
        free(table->targets[i].hooks);
    }
    free(table->targets);
    if (table == defaultTable) defaultTable = NULL;
    free(table);
}






typedef struct GotPatch
{
    char *objectPath;
    char *symbolName;
    void *original;
    struct GotPatch *next;
} GotPatch;

static GotPatch *gotPatches = NULL;

static GotPatch *got_patch_find(const char *path, const char *symbolName)
{
    for (GotPatch *patch = gotPatches; patch != NULL; patch = patch->next)
        if (strcmp(patch->objectPath, path) == 0 && strcmp(patch->symbolName, symbolName) == 0)
            return patch;

    return NULL;
}

static GotPatch *got_patch_record(const char *path, const char *symbolName, void *original)
{
    GotPatch *patch = calloc(1, sizeof(GotPatch));
    if (patch == NULL) return NULL;
    patch->objectPath = strdup(path);
    patch->symbolName = strdup(symbolName);
    patch->original = original;
    patch->next = gotPatches;
    gotPatches = patch;
    return patch;
}

void *LIBMATTI_MIXIN_GotOriginal(const char *path, const char *symbolName)
{
    GotPatch *patch = got_patch_find(path, symbolName);
    return patch != NULL ? patch->original : NULL;
}




static int make_writable(void *address)
{
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) return -1;
    uintptr_t page = (uintptr_t) address & ~((uintptr_t) pageSize - 1);
    return mprotect((void *) page, (size_t) pageSize, PROT_READ | PROT_WRITE);
}





static void got_patch_relocations(FILE *file, const Elf64_Shdr *relaSection, const Elf64_Sym *symbols,
                                  const char *strtab, unsigned char *base, const char *path,
                                  const char *const *symbolNames, size_t symbolCount,
                                  MattiMixinHandler hook, size_t *rewritten)
{
    size_t relaCount = (size_t) (relaSection->sh_size / sizeof(Elf64_Rela));
    Elf64_Rela *relas = malloc((size_t) relaSection->sh_size);
    if (relas == NULL) return;
    if (fseek(file, (long) relaSection->sh_offset, SEEK_SET) != 0 ||
        fread(relas, sizeof(Elf64_Rela), relaCount, file) != relaCount)
    {
        free(relas);
        return;
    }

    for (size_t k = 0; k < relaCount; k++)
    {
        if (ELF64_R_TYPE(relas[k].r_info) != R_X86_64_JUMP_SLOT &&
            ELF64_R_TYPE(relas[k].r_info) != R_X86_64_GLOB_DAT)
            continue;

        const char *symbolName = strtab + symbols[ELF64_R_SYM(relas[k].r_info)].st_name;
        int wanted = 0;
        for (size_t s = 0; s < symbolCount; s++)
            if (strcmp(symbolNames[s], symbolName) == 0) wanted = 1;
        if (!wanted) continue;

        void **slot = (void **) (base + relas[k].r_offset);
        if (got_patch_find(path, symbolName) == NULL)
        {
            if (got_patch_record(path, symbolName, *slot) == NULL) continue;
        }




        if (make_writable(slot) != 0) continue;
        *slot = *(void **) &hook;
        (*rewritten)++;
    }

    free(relas);
}

size_t LIBMATTI_MIXIN_GotPatch(const char *path, const char *const *symbolNames, size_t symbolCount,
                               MattiMixinHandler hook, void *userdata, const char *modId)
{
    (void) userdata;
    (void) modId;
    if (path == NULL || symbolNames == NULL || symbolCount == 0 || hook == NULL) return 0;

    void *handle = object_handle(path);
    if (handle == NULL) return 0;
    unsigned char *base = NULL;
    if (!object_load_bias(handle, path, &base))
    {
        dlclose(handle);
        return 0;
    }

    size_t rewritten = 0;
    FILE *file = fopen(object_file_path(path), "rb");
    if (file != NULL)
    {
        Elf64_Ehdr ehdr;
        if (fread(&ehdr, sizeof(ehdr), 1, file) == 1 && ehdr.e_ident[4] == ELFCLASS64 &&
            ehdr.e_shoff != 0 && ehdr.e_shnum != 0 && ehdr.e_shstrndx < ehdr.e_shnum)
        {
            Elf64_Shdr *sections = calloc(ehdr.e_shnum, sizeof(Elf64_Shdr));
            if (sections != NULL &&
                fseek(file, (long) ehdr.e_shoff, SEEK_SET) == 0 &&
                fread(sections, sizeof(Elf64_Shdr), ehdr.e_shnum, file) == ehdr.e_shnum)
            {
                for (uint16_t i = 0; i < ehdr.e_shnum && rewritten < symbolCount; i++)
                {
                    const Elf64_Shdr *relaSection = &sections[i];
                    if (relaSection->sh_type != SHT_RELA) continue;





                    const Elf64_Shdr *symtab = &sections[relaSection->sh_link];
                    if (symtab->sh_type != SHT_DYNSYM) continue;
                    const Elf64_Shdr *strtabHeader = &sections[symtab->sh_link];

                    Elf64_Sym *symbols = malloc((size_t) symtab->sh_size);
                    char *strtab = malloc((size_t) strtabHeader->sh_size + 1);
                    if (symbols != NULL && strtab != NULL &&
                        fseek(file, (long) symtab->sh_offset, SEEK_SET) == 0 &&
                        fread(symbols, sizeof(Elf64_Sym),
                              (size_t) (symtab->sh_size / sizeof(Elf64_Sym)), file) ==
                              symtab->sh_size / sizeof(Elf64_Sym) &&
                        fseek(file, (long) strtabHeader->sh_offset, SEEK_SET) == 0 &&
                        fread(strtab, 1, (size_t) strtabHeader->sh_size, file) ==
                                strtabHeader->sh_size)
                    {
                        strtab[strtabHeader->sh_size] = '\0';
                        got_patch_relocations(file, relaSection, symbols, strtab, base, path,
                                              symbolNames, symbolCount, hook, &rewritten);
                    }
                    free(symbols);
                    free(strtab);
                }
            }
            free(sections);
        }
        fclose(file);
    }




    dlclose(handle);
    return rewritten;
}

#endif

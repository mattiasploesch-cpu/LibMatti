// Port of net.minecraft.server.packs.repository.PackRepository.

#include "libmatti/net/minecraft/server/packs/repository/PackRepository.h"

#include <stdlib.h>
#include <string.h>

static void add_available(LIBMATTI_MC_PackRepository *repository, Pack *pack)
{
    // Java: Pack previous = available.putIfAbsent(pack.getId(), pack);
    //       if (previous != null) LOGGER.warn("Pack {} was already there, skipping", ...)
    for (size_t i = 0; i < repository->availableCount; i++)
    {
        if (strcmp(repository->availableIds[i], pack->location->id) == 0)
        {
            LIBMATTI_MC_Pack_Free(pack);
            return;
        }
    }
    repository->availableIds =
        realloc(repository->availableIds, (repository->availableCount + 1) * sizeof(char *));
    repository->availablePacks =
        realloc(repository->availablePacks, (repository->availableCount + 1) * sizeof(Pack *));
    repository->availableIds[repository->availableCount] = strdup(pack->location->id);
    repository->availablePacks[repository->availableCount] = pack;
    repository->availableCount++;
}

static int availability_comparator(const void *a, const void *b)
{
    const Pack *packA = *(const Pack *const *) a;
    const Pack *packB = *(const Pack *const *) b;
    return LIBMATTI_MC_Pack_CompareTo(packA, packB);
}

// Java: RepositorySource.loadPacks(Consumer<Pack>) - the consumer adapter
static void load_packs_consumer(void *userData, Pack *pack)
{
    add_available((LIBMATTI_MC_PackRepository *) userData, pack);
}

LIBMATTI_MC_PackRepository *LIBMATTI_MC_PackRepository_NewWithReorder(
    const LIBMATTI_MC_RepositorySource *const *sources, size_t sourceCount, void *rebuildConfigUserData,
    int (*reorder)(void *userData, Pack *const *packs, size_t count, Pack ***outReordered, size_t *outReorderedCount))
{
    LIBMATTI_MC_PackRepository *repository = calloc(1, sizeof(LIBMATTI_MC_PackRepository));
    repository->sources = sourceCount > 0 ? malloc(sourceCount * sizeof(LIBMATTI_MC_RepositorySource *)) : NULL;
    memcpy(repository->sources, sources, sourceCount * sizeof(LIBMATTI_MC_RepositorySource *));
    repository->sourceCount = sourceCount;
    repository->rebuildConfigUserData = rebuildConfigUserData;
    repository->reorder = reorder;
    return repository;
}

LIBMATTI_MC_PackRepository *LIBMATTI_MC_PackRepository_New(const LIBMATTI_MC_RepositorySource *const *sources,
                                                           size_t sourceCount)
{
    return LIBMATTI_MC_PackRepository_NewWithReorder(sources, sourceCount, NULL, NULL);
}

void LIBMATTI_MC_PackRepository_AddSource(LIBMATTI_MC_PackRepository *repository, LIBMATTI_MC_RepositorySource *source)
{
    repository->sources =
        realloc(repository->sources, (repository->sourceCount + 1) * sizeof(LIBMATTI_MC_RepositorySource *));
    repository->sources[repository->sourceCount] = source;
    repository->sourceCount++;
}

void LIBMATTI_MC_PackRepository_Reload(LIBMATTI_MC_PackRepository *repository)
{
    // Java: public void reload() {
    //   this.clearSelected();
    //   this.available.clear();
    //   for (RepositorySource source : this.sources) source.loadPacks(this::addPack);
    // ... then sorts the list
    for (size_t i = 0; i < repository->availableCount; i++)
    {
        free(repository->availableIds[i]);
        LIBMATTI_MC_Pack_Free(repository->availablePacks[i]);
    }
    free(repository->availableIds);
    free(repository->availablePacks);
    repository->availableIds = NULL;
    repository->availablePacks = NULL;
    repository->availableCount = 0;

    for (size_t i = 0; i < repository->sourceCount; i++)
    {
        repository->sources[i]->loadPacks(repository->sources[i], repository, load_packs_consumer);
    }

    // Java: this.available.values().stream().sorted(...).forEach(p -> this.available.put(p.getId(), p));
    if (repository->availableCount > 1)
    {
        qsort(repository->availablePacks, repository->availableCount, sizeof(Pack *), availability_comparator);
        for (size_t i = 0; i < repository->availableCount; i++)
        {
            free(repository->availableIds[i]);
            repository->availableIds[i] = strdup(repository->availablePacks[i]->location->id);
        }
    }
}

// Java: private void setSelected(List<String> ids) - keeps required packs, resolves in order
static void set_selected_internal(LIBMATTI_MC_PackRepository *repository, const char *const *ids, size_t count)
{
    for (size_t i = 0; i < repository->selectedCount; i++)
    {
        free(repository->selectedIds[i]);
    }
    free(repository->selectedIds);
    free(repository->selectedPacks);
    repository->selectedIds = NULL;
    repository->selectedPacks = NULL;
    repository->selectedCount = 0;

    // Java: resolve the ids in order, then append the required packs that are missing
    for (size_t i = 0; i < count; i++)
    {
        Pack *pack = LIBMATTI_MC_PackRepository_GetPack(repository, ids[i]);
        if (pack == NULL) continue;
        repository->selectedIds = realloc(repository->selectedIds, (repository->selectedCount + 1) * sizeof(char *));
        repository->selectedPacks =
            realloc(repository->selectedPacks, (repository->selectedCount + 1) * sizeof(Pack *));
        repository->selectedIds[repository->selectedCount] = strdup(ids[i]);
        repository->selectedPacks[repository->selectedCount] = pack;
        repository->selectedCount++;
    }
    for (size_t i = 0; i < repository->availableCount; i++)
    {
        Pack *pack = repository->availablePacks[i];
        if (!LIBMATTI_MC_Pack_IsRequired(pack)) continue;
        int alreadySelected = 0;
        for (size_t j = 0; j < repository->selectedCount; j++)
        {
            if (repository->selectedPacks[j] == pack)
            {
                alreadySelected = 1;
                break;
            }
        }
        if (alreadySelected) continue;
        repository->selectedIds = realloc(repository->selectedIds, (repository->selectedCount + 1) * sizeof(char *));
        repository->selectedPacks =
            realloc(repository->selectedPacks, (repository->selectedCount + 1) * sizeof(Pack *));
        repository->selectedIds[repository->selectedCount] = strdup(pack->location->id);
        repository->selectedPacks[repository->selectedCount] = pack;
        repository->selectedCount++;
    }
}

void LIBMATTI_MC_PackRepository_SetSelected(LIBMATTI_MC_PackRepository *repository, const char *const *ids,
                                            size_t count)
{
    set_selected_internal(repository, ids, count);
}

const char *const *LIBMATTI_MC_PackRepository_GetSelectedIds(const LIBMATTI_MC_PackRepository *repository,
                                                             size_t *outCount)
{
    *outCount = repository->selectedCount;
    return (const char *const *) repository->selectedIds;
}

Pack *const *LIBMATTI_MC_PackRepository_GetSelectedPacks(const LIBMATTI_MC_PackRepository *repository, size_t *outCount)
{
    *outCount = repository->selectedCount;
    return (Pack *const *) repository->selectedPacks;
}

Pack *const *LIBMATTI_MC_PackRepository_GetAvailablePacks(const LIBMATTI_MC_PackRepository *repository,
                                                          size_t *outCount)
{
    *outCount = repository->availableCount;
    return (Pack *const *) repository->availablePacks;
}

const char *const *LIBMATTI_MC_PackRepository_GetAvailableIds(const LIBMATTI_MC_PackRepository *repository,
                                                              size_t *outCount)
{
    *outCount = repository->availableCount;
    return (const char *const *) repository->availableIds;
}

Pack *LIBMATTI_MC_PackRepository_GetPack(const LIBMATTI_MC_PackRepository *repository, const char *id)
{
    for (size_t i = 0; i < repository->availableCount; i++)
    {
        if (strcmp(repository->availableIds[i], id) == 0) return repository->availablePacks[i];
    }
    return NULL;
}

int LIBMATTI_MC_PackRepository_IsAvailable(const LIBMATTI_MC_PackRepository *repository, const char *id)
{
    return LIBMATTI_MC_PackRepository_GetPack(repository, id) != NULL;
}

void LIBMATTI_MC_PackRepository_Free(LIBMATTI_MC_PackRepository *repository)
{
    for (size_t i = 0; i < repository->availableCount; i++)
    {
        free(repository->availableIds[i]);
        LIBMATTI_MC_Pack_Free(repository->availablePacks[i]);
    }
    free(repository->availableIds);
    free(repository->availablePacks);
    for (size_t i = 0; i < repository->selectedCount; i++)
        free(repository->selectedIds[i]);
    free(repository->selectedIds);
    free(repository->selectedPacks);
    free(repository->sources);
    free(repository);
}

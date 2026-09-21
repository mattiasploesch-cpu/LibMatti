// Port of net.minecraft.server.packs.repository.RepositorySource and
// net.minecraft.server.packs.repository.PackRepository.

#ifndef MATTICRAFT_MC_PACKS_REPOSITORY_PACKREPOSITORY_H
#define MATTICRAFT_MC_PACKS_REPOSITORY_PACKREPOSITORY_H

#include "libmatti/net/minecraft/server/packs/repository/Pack.h"

#include <stddef.h>

// Java: interface RepositorySource
typedef struct LIBMATTI_MC_RepositorySource
{
    // Java: void loadPacks(Consumer<Pack>)
    void (*loadPacks)(const struct LIBMATTI_MC_RepositorySource *self, void *userData,
                      void (*onPack)(void *userData, Pack *pack));
} LIBMATTI_MC_RepositorySource;

typedef struct LIBMATTI_MC_PackRepository
{
    // Java: private final Set<RepositorySource> sources
    LIBMATTI_MC_RepositorySource **sources;
    size_t sourceCount;
    // Java: private final Map<String, Pack> available
    char **availableIds;
    Pack **availablePacks;
    size_t availableCount;
    // Java: private final Map<String, Pack> selected
    char **selectedIds;
    Pack **selectedPacks;
    size_t selectedCount;
    // Java: private final UnaryOperator<Map<String, Pack>> rebuildConfig
    // The port keeps the comparator callback; NULL uses Pack.compareTo.
    void *rebuildConfigUserData;
    int (*reorder)(void *userData, Pack *const *packs, size_t count, Pack ***outReordered, size_t *outReorderedCount);
} LIBMATTI_MC_PackRepository;

// Java: public PackRepository(RepositorySource... sources)
LIBMATTI_MC_PackRepository *LIBMATTI_MC_PackRepository_New(const LIBMATTI_MC_RepositorySource *const *sources,
                                                           size_t sourceCount);
// NeoForge constructor with the rebuild-config operator; pass NULL for vanilla behaviour
LIBMATTI_MC_PackRepository *LIBMATTI_MC_PackRepository_NewWithReorder(
    const LIBMATTI_MC_RepositorySource *const *sources, size_t sourceCount,
    void *rebuildConfigUserData,
    int (*reorder)(void *userData, Pack *const *packs, size_t count, Pack ***outReordered, size_t *outReorderedCount));

// Java: public void addSource(RepositorySource)
void LIBMATTI_MC_PackRepository_AddSource(LIBMATTI_MC_PackRepository *repository, LIBMATTI_MC_RepositorySource *source);

// Java: public void reload()
void LIBMATTI_MC_PackRepository_Reload(LIBMATTI_MC_PackRepository *repository);

// Java: public void setSelected(Collection<String> ids)
void LIBMATTI_MC_PackRepository_SetSelected(LIBMATTI_MC_PackRepository *repository, const char *const *ids,
                                            size_t count);
// Java: public Collection<String> getSelectedIds()
const char *const *LIBMATTI_MC_PackRepository_GetSelectedIds(const LIBMATTI_MC_PackRepository *repository,
                                                             size_t *outCount);
// Java: public Collection<Pack> getSelectedPacks()
Pack *const *LIBMATTI_MC_PackRepository_GetSelectedPacks(const LIBMATTI_MC_PackRepository *repository,
                                                         size_t *outCount);
// Java: public Collection<Pack> getAvailablePacks()
Pack *const *LIBMATTI_MC_PackRepository_GetAvailablePacks(const LIBMATTI_MC_PackRepository *repository,
                                                          size_t *outCount);
// Java: public Collection<String> getAvailableIds()
const char *const *LIBMATTI_MC_PackRepository_GetAvailableIds(const LIBMATTI_MC_PackRepository *repository,
                                                              size_t *outCount);
// Java: @Nullable public Pack getPack(String id) - NULL when absent
Pack *LIBMATTI_MC_PackRepository_GetPack(const LIBMATTI_MC_PackRepository *repository, const char *id);
// Java: public boolean isAvailable(String id)
int LIBMATTI_MC_PackRepository_IsAvailable(const LIBMATTI_MC_PackRepository *repository, const char *id);

void LIBMATTI_MC_PackRepository_Free(LIBMATTI_MC_PackRepository *repository);

#endif

// Port of net.neoforged.neoforge.client.loading.ClientModLoader.
// The game calls begin() right after Bootstrap.validate(), finish() when the resource pack
// repository is set up and completeModLoading() where Minecraft expects the reload runnable.

#ifndef MATTICRAFT_NEOFORGE_CLIENT_LOADING_CLIENTMODLOADER_H
#define MATTICRAFT_NEOFORGE_CLIENT_LOADING_CLIENTMODLOADER_H

#include <stddef.h>

// Java: public static void begin()
void LIBMATTI_NEOFORGE_ClientModLoader_Begin(void);

// Java: public static void finish(final PackRepository defaultResourcePacks, final ReloadableResourceManager mcResourceManager)
// Both arguments are game-side objects the port cannot name yet; NULL is the game port's stand-in.
void LIBMATTI_NEOFORGE_ClientModLoader_Finish(void *defaultResourcePacks, void *mcResourceManager);

// Java: public static CompletableFuture<Void> onResourceReload(PreparableReloadListener.SharedState sharedState,
//         Executor asyncExecutor, PreparationBarrier stage, Executor syncExecutor)
// Java's future chain (startModLoading, barrier wait, finishModLoading) runs in order here.
void LIBMATTI_NEOFORGE_ClientModLoader_OnResourceReload(void);

// Java: public static Runnable completeModLoading(Runnable initialScreensTask) - the port returns the
// runnable it was handed (the error-screen branch is the game port's part).
void (*LIBMATTI_NEOFORGE_ClientModLoader_CompleteModLoading(void (*initialScreensTask)(void *userdata)))(void *userdata);

// Java: static boolean isLoading()
int LIBMATTI_NEOFORGE_ClientModLoader_IsLoading(void);

#endif //MATTICRAFT_NEOFORGE_CLIENT_LOADING_CLIENTMODLOADER_H

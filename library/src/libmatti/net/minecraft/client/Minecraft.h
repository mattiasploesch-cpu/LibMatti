// Port of net.minecraft.client.Minecraft (the game skeleton: window ownership,
// the run loop, tick + render split, stop/shutdown). Everything behind the loop
// (screens, entities, renderer internals) is the game port itself and stays out.
//
// Java fields the skeleton carries: window, deltaTracker, running/stopped,
// frames, gameThread, launchedVersion/versionType, pause, noRender, smoothCam.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_MINECRAFT_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_MINECRAFT_H

#include "libmatti/net/minecraft/client/DeltaTracker.h"
#include "libmatti/net/minecraft/client/main/GameConfig.h"

#include <stddef.h>

typedef struct LIBMATTI_MC_Minecraft LIBMATTI_MC_Minecraft;
typedef struct LIBMATTI_MC_TextureManager LIBMATTI_MC_TextureManager;

// Java: public TextureManager getTextureManager()

// Java: this::getTickTargetMillis (the default is the plain ms per tick)
float LIBMATTI_MC_Minecraft_GetTickTargetMillis(float msPerTick);

// Java: public Minecraft(GameConfig) - window init runs through the
// RunGameLoop skeleton; returns NULL when the window could not be created.
LIBMATTI_MC_Minecraft *LIBMATTI_MC_Minecraft_New(const LIBMATTI_MC_GameConfig *config);

// Java: public void run() - while (running) { tick, render, yield }
void LIBMATTI_MC_Minecraft_Run(LIBMATTI_MC_Minecraft *minecraft);

// Java: public void stop() / public boolean isRunning()
void LIBMATTI_MC_Minecraft_Stop(LIBMATTI_MC_Minecraft *minecraft);
int LIBMATTI_MC_Minecraft_IsRunning(const LIBMATTI_MC_Minecraft *minecraft);

// Java: public void destroy() - the shutdown path after the loop exits
void LIBMATTI_MC_Minecraft_Destroy(LIBMATTI_MC_Minecraft *minecraft);

// Java: public static Minecraft getInstance()
LIBMATTI_MC_Minecraft *LIBMATTI_MC_Minecraft_GetInstance(void);

// Java: public Window getWindow() - the long is the native GLFW window handle
long LIBMATTI_MC_Minecraft_GetWindow(const LIBMATTI_MC_Minecraft *minecraft);
LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_Minecraft_GetDeltaTracker(const LIBMATTI_MC_Minecraft *minecraft);
LIBMATTI_MC_TextureManager *LIBMATTI_MC_Minecraft_GetTextureManager(const LIBMATTI_MC_Minecraft *minecraft);

// Java: public String getLaunchedVersion() / getVersionType()
const char *LIBMATTI_MC_Minecraft_GetLaunchedVersion(const LIBMATTI_MC_Minecraft *minecraft);
const char *LIBMATTI_MC_Minecraft_GetVersionType(const LIBMATTI_MC_Minecraft *minecraft);

// Java: public int getFps() / getFrames() (debug overlay reads them)
int LIBMATTI_MC_Minecraft_GetFps(const LIBMATTI_MC_Minecraft *minecraft);
int LIBMATTI_MC_Minecraft_GetFrames(const LIBMATTI_MC_Minecraft *minecraft);

// Java: public boolean noRender / public void pauseGame(boolean) - pause only
// toggles the tracker state in the skeleton (no screens yet).
int LIBMATTI_MC_Minecraft_NoRender(const LIBMATTI_MC_Minecraft *minecraft);
void LIBMATTI_MC_Minecraft_SetNoRender(LIBMATTI_MC_Minecraft *minecraft, int noRender);

#endif //MATTICRAFT_NET_MINECRAFT_CLIENT_MINECRAFT_H

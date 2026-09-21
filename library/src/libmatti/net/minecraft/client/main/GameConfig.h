// Port of net.minecraft.client.main.GameConfig (and the blaze3d DisplayData it carries).
// Java's nested classes become plain structs; OptionalInt becomes a value plus a has flag.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_GAMECONFIG_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_GAMECONFIG_H

// Java: com.mojang.blaze3d.platform.DisplayData(int width, int height,
// OptionalInt fullscreenWidth, OptionalInt fullscreenHeight, boolean isFullscreen)
typedef struct LIBMATTI_MC_DisplayData
{
    int width;
    int height;
    int fullscreenWidth;
    int fullscreenWidthPresent;
    int fullscreenHeight;
    int fullscreenHeightPresent;
    int isFullscreen;
} LIBMATTI_MC_DisplayData;

// Java: GameConfig.GameData(boolean demo, String launchVersion, String versionType,
// boolean disableMultiplayer, boolean disableChat, ...)
typedef struct LIBMATTI_MC_GameConfigGameData
{
    int demo;
    const char *launchVersion;
    const char *versionType;
    int disableMultiplayer;
    int disableChat;
} LIBMATTI_MC_GameConfigGameData;

// Java: GameConfig.UserData(User user, Proxy proxy) - the port keeps the user fields
// (name, uuid, accessToken) and drops the proxy (no network stack in the skeleton).
typedef struct LIBMATTI_MC_GameConfigUserData
{
    const char *name;
    const char *uuid;
    const char *accessToken;
} LIBMATTI_MC_GameConfigUserData;

// Java: GameConfig.FolderData(File gameDirectory, File resourcePackDirectory,
// File assetDirectory, String assetIndex)
typedef struct LIBMATTI_MC_GameConfigFolderData
{
    const char *gameDirectory;
    const char *resourcePackDirectory;
    const char *assetDirectory;
    const char *assetIndex;
} LIBMATTI_MC_GameConfigFolderData;

// Java: public final GameConfig.UserData user; DisplayData display;
// FolderData location; GameData game; QuickPlayData quickPlay;
// Quick Play is recorded (Java: QuickPlayData(logPath, variant)) but unused by the skeleton.
typedef struct LIBMATTI_MC_GameConfig
{
    LIBMATTI_MC_GameConfigUserData user;
    LIBMATTI_MC_DisplayData display;
    LIBMATTI_MC_GameConfigFolderData location;
    LIBMATTI_MC_GameConfigGameData game;
    const char *quickPlayLogPath;
    const char *quickPlayWorld;
} LIBMATTI_MC_GameConfig;

#endif //MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_GAMECONFIG_H

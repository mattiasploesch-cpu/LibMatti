// Port of net.minecraft.client.Options - the game settings store. Java keeps
// the field set on the Options instance and saves the plain `key:value` lines
// through Options.save (PrintStream over the options.txt in the game folder);
// the port mirrors the file format 1:1 (NOT TOML - the vanilla options.txt
// predates the config system) and rides FMLPaths.GAMEDIR for the location.
// The P6.4 slice carries the settings the pause/options screens drive: the
// mouse sensitivity, the master volume, the auto gui scale and the fullscreen
// toggle (the render loop re-reads it every frame like the window events do).

#ifndef MATTICRAFT_MC_CLIENT_OPTIONS_H
#define MATTICRAFT_MC_CLIENT_OPTIONS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_FML_IConfigSpec;

// Java: public static final double DEFAULT_MOUSE_SENSITIVITY = 0.5
#define LIBMATTI_MC_Options_DEFAULT_SENSITIVITY 0.5f
// Java: the master volume OptionInstance default (10% steps land on 1.0)
#define LIBMATTI_MC_Options_DEFAULT_VOLUME 1.0f

typedef struct LIBMATTI_MC_Options
{
    // Java: private double sensitivity = 0.5 (the 0..1 option value)
    float sensitivity;
    // Java: private double masterVolume = 1.0 (SoundSource.MASTER slider)
    float masterVolume;
    // Java: private int guiScale = 0 - 0 = auto (Window.calculateScale drives)
    int guiScale;
    // Java: private boolean fullscreen
    bool fullscreen;
} LIBMATTI_MC_Options;

// Java: public Options(Minecraft) - the defaults; load() rides the caller
// (the client loads right after the FML paths exist)
void LIBMATTI_MC_Options_Init(LIBMATTI_MC_Options *options);
void LIBMATTI_MC_Options_Free(LIBMATTI_MC_Options *options);

// Java: public void load() - reads the `key:value` lines of options.txt in
// the GAMEDIR; unknown keys stay ignored (Java's noisy trace), missing file
// keeps the defaults. The parse maps the option data-codes the save writes
// (sensitivity, masterVolume, guiScale, fullscreen).
void LIBMATTI_MC_Options_Load(LIBMATTI_MC_Options *options);
// Java: public void save() - writes the file back with the current values
// (the flush the pause screen runs on close).
void LIBMATTI_MC_Options_Save(const LIBMATTI_MC_Options *options);
// the absolute options.txt path (FMLPaths.GAMEDIR resolve; the caller frees)
char *LIBMATTI_MC_Options_Path(void);

// Java: public double sensitivity().get() / set(...) - the 0..1 option value
float LIBMATTI_MC_Options_GetSensitivity(const LIBMATTI_MC_Options *options);
void LIBMATTI_MC_Options_SetSensitivity(LIBMATTI_MC_Options *options, float value);
// Java: the slider's 0..200% label (the widget message rides it)
int LIBMATTI_MC_Options_SensitivityPercent(const LIBMATTI_MC_Options *options);
// Java: public double masterVolume... - clamped to the 0..1 slider range
float LIBMATTI_MC_Options_GetMasterVolume(const LIBMATTI_MC_Options *options);
void LIBMATTI_MC_Options_SetMasterVolume(LIBMATTI_MC_Options *options, float value);
// Java: Window.calculateScale over the auto flag - the port keeps the calc
// inside Minecraft.c; this exposes the stored scale (0 = auto)
int LIBMATTI_MC_Options_GetGuiScale(const LIBMATTI_MC_Options *options);
void LIBMATTI_MC_Options_SetGuiScale(LIBMATTI_MC_Options *options, int value);
bool LIBMATTI_MC_Options_IsFullscreen(const LIBMATTI_MC_Options *options);
void LIBMATTI_MC_Options_SetFullscreen(LIBMATTI_MC_Options *options, bool value);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_OPTIONS_H

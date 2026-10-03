// Port of net.minecraft.client.Options - the game settings store. The file
// side mirrors Java's Options.save exactly: one `key:value` line per option
// (sensitivity, masterVolume, guiScale, fullscreen), written with the plain
// key names Java's option data-codes serialize. The load ignores unknown
// keys (Java traces them and moves on) and keeps the defaults when the file
// is missing - the first save creates it through FMLPaths.GAMEDIR.

#include "libmatti/net/minecraft/client/Options.h"

#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public void load() - BufferedInputStream over options.txt, the
// GenericLineReader splits on the first ':', loadBoolean/applyParse map the
// option keys; unknown lines land in the trace (the port drops them).
void LIBMATTI_MC_Options_Load(LIBMATTI_MC_Options *options)
{
    if (options == NULL)
        return;
    char *path = LIBMATTI_MC_Options_Path();
    if (path == NULL)
        return;
    FILE *in = fopen(path, "r");
    if (in == NULL)
    {
        // Java: the missing file keeps the defaults (save() creates it)
        free(path);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), in) != NULL)
    {
        // Java: GenericLineReader.split - the first ':' separates key/value
        char *colon = strchr(line, ':');
        if (colon == NULL)
            continue;
        *colon = '\0';
        const char *key = line;
        char *value = colon + 1;
        // Java: strip the line terminator the reader trims (the value must
        // terminate BEFORE the strcmp/parse checks ride it)
        size_t len = strlen(value);
        while (len > 0 && (value[len - 1] == '\n' || value[len - 1] == '\r'))
            value[--len] = '\0';
        if (strcmp(key, "sensitivity") == 0)
        {
            char *end = NULL;
            float parsed = strtof(value, &end);
            if (end != value && *end == '\0')
            {
                options->sensitivity = parsed;
                if (options->sensitivity < 0.0f) options->sensitivity = 0.0f;
                if (options->sensitivity > 1.0f) options->sensitivity = 1.0f;
            }
            // Java: the unparseable value keeps the current (default) one
        }
        else if (strcmp(key, "masterVolume") == 0)
        {
            char *end = NULL;
            float parsed = strtof(value, &end);
            if (end != value && *end == '\0')
            {
                options->masterVolume = parsed;
                if (options->masterVolume < 0.0f) options->masterVolume = 0.0f;
                if (options->masterVolume > 1.0f) options->masterVolume = 1.0f;
            }
        }
        else if (strcmp(key, "guiScale") == 0)
        {
            char *end = NULL;
            long parsed = strtol(value, &end, 10);
            if (end != value && *end == '\0')
            {
                options->guiScale = (int) parsed;
                if (options->guiScale < 0) options->guiScale = 0;
                if (options->guiScale > 8) options->guiScale = 8;
            }
        }
        else if (strcmp(key, "fullscreen") == 0)
        {
            options->fullscreen = strcmp(value, "true") == 0;
        }
        // Java: the unknown keys ride the noisy trace and stay ignored
    }
    fclose(in);
    free(path);
}

// Java: public void save() - PrintStream.println("sensitivity:" + ...) over
// the same set of options every time (the file carries the full option set).
void LIBMATTI_MC_Options_Save(const LIBMATTI_MC_Options *options)
{
    if (options == NULL)
        return;
    char *path = LIBMATTI_MC_Options_Path();
    if (path == NULL)
        return;
    FILE *out = fopen(path, "w");
    if (out != NULL)
    {
        // Java: the option order follows the field set (the write order is
        // not load-bearing; the load accepts any order)
        fprintf(out, "sensitivity:%g\n", (double) options->sensitivity);
        fprintf(out, "masterVolume:%g\n", (double) options->masterVolume);
        fprintf(out, "guiScale:%d\n", options->guiScale);
        fprintf(out, "fullscreen:%s\n", options->fullscreen ? "true" : "false");
        fclose(out);
    }
    free(path);
}

char *LIBMATTI_MC_Options_Path(void)
{
    // Java: this.optionsFile = new File(FMLPaths.GAMEDIR.get().toFile(), "options.txt")
    const char *gameDir = LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_GAMEDIR);
    if (gameDir == NULL)
        return NULL;
    size_t size = strlen(gameDir) + sizeof("/options.txt");
    char *path = malloc(size);
    if (path != NULL)
        snprintf(path, size, "%s/options.txt", gameDir);
    return path;
}

// Java: public Options(Minecraft) - the field defaults
void LIBMATTI_MC_Options_Init(LIBMATTI_MC_Options *options)
{
    if (options == NULL)
        return;
    options->sensitivity = LIBMATTI_MC_Options_DEFAULT_SENSITIVITY;
    options->masterVolume = LIBMATTI_MC_Options_DEFAULT_VOLUME;
    options->guiScale = 0; // Java: 0 = auto
    options->fullscreen = false;
}

void LIBMATTI_MC_Options_Free(LIBMATTI_MC_Options *options)
{
    (void) options; // the struct rides the Minecraft allocation
}

float LIBMATTI_MC_Options_GetSensitivity(const LIBMATTI_MC_Options *options)
{
    return options != NULL ? options->sensitivity : LIBMATTI_MC_Options_DEFAULT_SENSITIVITY;
}

void LIBMATTI_MC_Options_SetSensitivity(LIBMATTI_MC_Options *options, float value)
{
    if (options == NULL)
        return;
    // Java: the OptionInstance clamps to its 0..1 slider range
    options->sensitivity = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

int LIBMATTI_MC_Options_SensitivityPercent(const LIBMATTI_MC_Options *options)
{
    if (options == NULL)
        return 100;
    // Java: the label carries one decimal (10% steps keep it exact)
    return (int) (options->sensitivity * 200.0f + 0.5f);
}

float LIBMATTI_MC_Options_GetMasterVolume(const LIBMATTI_MC_Options *options)
{
    return options != NULL ? options->masterVolume : LIBMATTI_MC_Options_DEFAULT_VOLUME;
}

void LIBMATTI_MC_Options_SetMasterVolume(LIBMATTI_MC_Options *options, float value)
{
    if (options == NULL)
        return;
    // Java: the slider clamps to 0..1
    options->masterVolume = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

int LIBMATTI_MC_Options_GetGuiScale(const LIBMATTI_MC_Options *options)
{
    return options != NULL ? options->guiScale : 0;
}

void LIBMATTI_MC_Options_SetGuiScale(LIBMATTI_MC_Options *options, int value)
{
    if (options == NULL)
        return;
    options->guiScale = value < 0 ? 0 : (value > 8 ? 8 : value);
}

bool LIBMATTI_MC_Options_IsFullscreen(const LIBMATTI_MC_Options *options)
{
    return options != NULL ? options->fullscreen : false;
}

void LIBMATTI_MC_Options_SetFullscreen(LIBMATTI_MC_Options *options, bool value)
{
    if (options == NULL)
        return;
    options->fullscreen = value;
}

// Port-only stand-in for NeoForge's ModConfigSpec (see the header).
// The behaviour follows SimpleConfigSpec from FML's ConfigTrackerTest:
//   correct(config, dryRun): "configEntry" must exist and be an Integer, else set 4;
//   comment "Test comment:" is added on the real run.
// The port's CommentedConfig has no comment storage yet (the parser does not read comments),
// so correct() only restores the value.

#include "libmatti/net/neoforged/fml/config/ModConfigSpec.h"

#include <stdlib.h>

// Java: private boolean correct(UnmodifiableCommentedConfig config, boolean dryRun)
static int correct_impl(const LIBMATTI_NC_Config *config, int dryRun)
{
    int ok = 1;
    const char *path[] = {"configEntry"};

    LIBMATTI_NC_Value *value = LIBMATTI_NC_Config_Get(config, path, 1);
    if (value == NULL || value->type != LIBMATTI_NC_VALUE_INT)
    {
        ok = 0;
        if (!dryRun)
        {
            LIBMATTI_NC_Value defaultValue = LIBMATTI_NC_Value_OfInt(4);
            LIBMATTI_NC_Config_Set((LIBMATTI_NC_Config *) config, path, 1, &defaultValue);
        }
    }
    // Java: if (!dryRun && !config.containsComment("configEntry")) setComment("configEntry", "Test comment:")
    return ok;
}

static int spec_is_empty(void *self)
{
    (void) self;
    return 0;
}

// Java: @Override public void validateSpec(ModConfig config) {}
static void spec_validate_spec(void *self, void *config)
{
    (void) self;
    (void) config;
}

// Java: @Override public boolean isCorrect(UnmodifiableCommentedConfig config) { return correct(config, true); }
static int spec_is_correct(void *self, const LIBMATTI_NC_Config *config)
{
    (void) self;
    return correct_impl(config, 1);
}

// Java: @Override public void correct(CommentedConfig config) { correct(config, false); }
static void spec_correct(void *self, LIBMATTI_NC_Config *config)
{
    (void) self;
    correct_impl(config, 0);
}

// Java: @Override public void acceptConfig(@Nullable ILoadedConfig config) {
//           loadedValue = config == null ? 4 : config.config().getInt("configEntry"); }
static void spec_accept_config(void *self, LIBMATTI_FML_ILoadedConfig *config)
{
    LIBMATTI_FML_ModConfigSpec *spec = self;

    if (config == NULL)
    {
        spec->loadedValue = 4;
        return;
    }

    LIBMATTI_NC_Value *value = LIBMATTI_NC_Config_GetKey(config->config(config->self), "configEntry");
    if (value != NULL && value->type == LIBMATTI_NC_VALUE_INT) spec->loadedValue = value->integer;
}

LIBMATTI_FML_ModConfigSpec *LIBMATTI_FML_ModConfigSpec_New(void)
{
    LIBMATTI_FML_ModConfigSpec *spec = calloc(1, sizeof(*spec));
    spec->loadedValue = 4;
    spec->spec = LIBMATTI_FML_IConfigSpec_Of(spec, spec_is_empty, spec_validate_spec, spec_is_correct, spec_correct,
                                             spec_accept_config);
    return spec;
}

void LIBMATTI_FML_ModConfigSpec_Free(LIBMATTI_FML_ModConfigSpec *spec)
{
    free(spec);
}

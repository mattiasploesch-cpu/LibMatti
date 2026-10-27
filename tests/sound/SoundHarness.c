// Sound harness: the P5.6 core - the SoundEngine stack over the
// LIBMATTI_OAL_* wrapper (library/channel/boot lifecycle, the headless
// no-op tolerance) and the SoundEvents resolve cache the SoundTypes ride.

#include "libmatti/net/minecraft/client/sounds/SoundEngine.h"
#include "libmatti/net/minecraft/world/level/block/SoundType.h"
#include "libmatti/org/lwjgl/openal/OAL.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

static void test_library_boot(void)
{
    // Java: Library.init - headless (no audio device) leaves available = 0;
    // the port's contract is that the library never crashes on teardown.
    LIBMATTI_MC_SoundLibrary *library = LIBMATTI_MC_SoundLibrary_New();
    check(library != NULL, "library allocates");
    if (library == NULL)
        return;

    int oal = LIBMATTI_OAL_IsAvailable();
    if (!oal)
    {
        check(library->available == 0, "headless boot leaves available = 0");
        check(library->sourceCount == 30, "the 30-source pool stays allocated");
        check(LIBMATTI_MC_SoundLibrary_HasLinearDistance(library) == 0,
              "linear distance off without the binding");
        LIBMATTI_MC_SoundLibrary_Close(library);
        LIBMATTI_MC_SoundLibrary_Free(library);
        return;
    }

    // Live binding: the device/context booted and the pool filled.
    check(library->available == 1, "live boot sets available = 1");
    check(library->device != 0, "device handle non-null");
    check(library->context != 0, "context handle non-null");
    check(library->sourceCount == 30, "DEFAULT_CHANNEL_COUNT = 30 sources");
    LIBMATTI_MC_SoundLibrary_Close(library);
    LIBMATTI_MC_SoundLibrary_Free(library);
}

static void test_engine_headless_noop(void)
{
    // Java: SoundEngine.reload/play/tick - every call must survive the dead
    // binding (the CI machines carry no audio device).
    LIBMATTI_MC_SoundEngine *engine = LIBMATTI_MC_SoundEngine_New();
    check(engine != NULL, "engine allocates");
    if (engine == NULL)
        return;

    LIBMATTI_MC_SoundEngine_Reload(engine);
    check(engine->library != NULL, "reload allocates the library");
    if (engine->library != NULL && !LIBMATTI_OAL_IsAvailable())
        check(engine->library->available == 0, "headless engine stays dead");

    LIBMATTI_MC_SoundEngine_SetVolume(engine, 0.5f);
    check(engine->masterVolume == 0.5f, "master volume stores");

    LIBMATTI_MC_ListenerTransform transform = {
        {0.0f, 64.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}};
    LIBMATTI_MC_SoundEngine_Tick(engine, &transform);

    // Java: play(LocalPlayer, SoundInstance) - unknown id and known id both
    // ride the dead binding silently.
    LIBMATTI_MC_SoundEngine_Play(engine, "block.stone.break", 1.0f, 64.0f, 2.0f, 1.0f, 1.0f);
    LIBMATTI_MC_SoundEngine_Play(engine, "not.a.sound", 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    LIBMATTI_MC_SoundEngine_PlayUi(engine, "block.stone.place", 1.0f, 1.0f);
    LIBMATTI_MC_SoundEngine_StopAll(engine);
    LIBMATTI_MC_SoundEngine_Pause(engine);
    LIBMATTI_MC_SoundEngine_Resume(engine);

    LIBMATTI_MC_SoundEngine_Free(engine);
}

static void test_channel_lifecycle(void)
{
    // Java: Channel(Library, staticPool) - a dead binding hands back the
    // uninitialized channel; live, the source allocates from the pool.
    LIBMATTI_MC_SoundLibrary *library = LIBMATTI_MC_SoundLibrary_New();
    check(library != NULL, "channel test library allocates");
    if (library == NULL)
        return;

    LIBMATTI_MC_SoundChannel *channel = LIBMATTI_MC_SoundChannel_New(library);
    check(channel != NULL, "channel allocates");
    if (channel == NULL)
    {
        LIBMATTI_MC_SoundLibrary_Free(library);
        return;
    }

    if (library->available)
    {
        check(channel->initialized, "live channel initializes");
        check(channel->source != 0, "live channel carries an AL source");
    }
    else
    {
        check(!channel->initialized, "dead binding leaves the channel uninitialized");
    }

    // Java: playing()/stopped() start false; the setters tolerate the dead
    // source handle (0) without crashing.
    check(!LIBMATTI_MC_SoundChannel_Playing(channel), "fresh channel not playing");
    check(!LIBMATTI_MC_SoundChannel_Stopped(channel), "fresh channel not stopped");
    LIBMATTI_MC_SoundChannel_SetSelfPosition(channel, 1.0f, 2.0f, 3.0f);
    LIBMATTI_MC_SoundChannel_SetPitch(channel, 0.8f);
    LIBMATTI_MC_SoundChannel_SetLooping(channel, 0);
    LIBMATTI_MC_SoundChannel_SetRelative(channel, 0);
    LIBMATTI_MC_SoundChannel_LinearAttenuation(channel, 16.0f);
    LIBMATTI_MC_SoundChannel_SetGain(channel, 0.5f);
    LIBMATTI_MC_SoundChannel_Play(channel);
    LIBMATTI_MC_SoundChannel_Pause(channel);
    LIBMATTI_MC_SoundChannel_Unpause(channel);
    LIBMATTI_MC_SoundChannel_Stop(channel);

    LIBMATTI_MC_SoundChannel_Free(channel);
    LIBMATTI_MC_SoundLibrary_Close(library);
    LIBMATTI_MC_SoundLibrary_Free(library);
}

static void test_sound_buffer(void)
{
    // Java: new SoundBuffer(data, format) - the port wraps the caller's
    // 16-bit samples and the format rides alongside.
    short samples[8] = {0, 1000, -1000, 3276, -3276, 500, -500, 0};
    LIBMATTI_MC_SoundBuffer *buffer = LIBMATTI_MC_SoundBuffer_New(samples, 8, 1, 22050);
    check(buffer != NULL, "buffer allocates");
    if (buffer == NULL)
        return;
    check(buffer->data == samples, "buffer wraps the caller's samples");
    check(buffer->sampleCount == 8, "sample count stores");
    check(buffer->channels == 1, "mono format stores");
    check(buffer->sampleRate == 22050, "sample rate stores");
    check(buffer->alBuffer == 0, "AL buffer uploads lazily");

    // Java: discardAlBuffer without a live context stays a no-op.
    LIBMATTI_MC_SoundBuffer_Free(buffer);
}

static void test_sound_events(void)
{
    // The registry synthesizes one tone per sound event id - the harness
    // checks the cache identity and the SoundType wiring.
    check(LIBMATTI_MC_SoundEvents_Count() > 0, "sound events registered");

    LIBMATTI_MC_SoundBuffer *first = LIBMATTI_MC_SoundEvents_Resolve("block.stone.break");
    check(first != NULL, "stone break resolves");
    LIBMATTI_MC_SoundBuffer *again = LIBMATTI_MC_SoundEvents_Resolve("block.stone.break");
    check(first == again, "resolve caches per id");

    LIBMATTI_MC_SoundBuffer *other = LIBMATTI_MC_SoundEvents_Resolve("block.wood.place");
    check(other != NULL && other != first, "different ids resolve different buffers");

    if (other != NULL)
    {
        check(other->channels == 1, "synthesized tones are mono");
        check(other->sampleRate == 22050, "synthesized tones run 22050 Hz");
        check(other->sampleCount > 0, "synthesized tones carry samples");
        check(other->data != NULL, "synthesized tones carry PCM");
    }

    // Java: SoundType.STONE.getBreakSound() - the SoundType ids resolve
    // through the same registry the engine plays from.
    const LIBMATTI_MC_SoundType *stone = LIBMATTI_MC_SoundType_STONE();
    check(stone != NULL, "stone SoundType exists");
    if (stone != NULL)
    {
        const char *breakId = LIBMATTI_MC_SoundType_GetBreakSound(stone);
        check(breakId != NULL && breakId[0] != '\0', "stone break sound id non-empty");
        LIBMATTI_MC_SoundBuffer *resolved = LIBMATTI_MC_SoundEvents_Resolve(breakId);
        check(resolved != NULL, "SoundType break id resolves");
    }

    // Unknown ids ride the "place" default tone (the registry's catch-all),
    // still cached like every other resolve.
    LIBMATTI_MC_SoundBuffer *fallback = LIBMATTI_MC_SoundEvents_Resolve("not.a.sound");
    check(fallback != NULL, "unknown id rides the default tone");
    check(fallback == LIBMATTI_MC_SoundEvents_Resolve("not.a.sound"),
          "unknown id caches too");
}

int main(void)
{
    test_library_boot();
    test_engine_headless_noop();
    test_channel_lifecycle();
    test_sound_buffer();
    test_sound_events();
    printf("sound: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}

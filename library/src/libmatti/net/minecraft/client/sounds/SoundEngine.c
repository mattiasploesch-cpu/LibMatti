// The sound engine implementation (see SoundEngine.h). Java's slice:
// blaze3d.audio (Library/Channel/Listener/SoundBuffer) + SoundEngine - the
// port drives the AL objects through the LIBMATTI_OAL_* wrapper only. A dead
// binding or a missing device leaves every path as a no-op (the headless
// machines keep booting; the harness checks the bookkeeping instead).

#include "libmatti/net/minecraft/client/sounds/SoundEngine.h"

#include "libmatti/org/lwjgl/openal/OAL.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// SoundBuffer
// ---------------------------------------------------------------------------

LIBMATTI_MC_SoundBuffer *LIBMATTI_MC_SoundBuffer_New(short *data, int sampleCount,
                                                      int channels, int sampleRate)
{
    if (data == NULL || sampleCount <= 0)
        return NULL;
    LIBMATTI_MC_SoundBuffer *buffer = calloc(1, sizeof(LIBMATTI_MC_SoundBuffer));
    if (buffer == NULL)
        return NULL;
    buffer->data = data;
    buffer->sampleCount = sampleCount;
    buffer->channels = channels < 2 ? 1 : 2;
    buffer->sampleRate = sampleRate > 0 ? sampleRate : 22050;
    buffer->alBuffer = 0;
    return buffer;
}

unsigned int LIBMATTI_MC_SoundBuffer_Upload(LIBMATTI_MC_SoundBuffer *buffer)
{
    if (buffer == NULL || buffer->data == NULL)
        return 0;
    if (buffer->alBuffer != 0)
        return buffer->alBuffer; // Java: getAlBuffer memoizes the handle.
    LIBMATTI_OAL_AL_GenBuffers(1, &buffer->alBuffer);
    if (buffer->alBuffer == 0)
        return 0;
    int format = buffer->channels == 2 ? LIBMATTI_OAL_AL_FORMAT_STEREO16
                                       : LIBMATTI_OAL_AL_FORMAT_MONO16;
    // Java: alBufferData(buffer, format, data, sampleRate) - the size rides
    // in bytes (16-bit = 2 bytes per sample).
    LIBMATTI_OAL_AL_BufferData(buffer->alBuffer, format, buffer->data,
                               buffer->sampleCount * (int) sizeof(short), buffer->sampleRate);
    if (LIBMATTI_OAL_AL_GetError() != LIBMATTI_OAL_AL_NO_ERROR)
    {
        LIBMATTI_OAL_AL_DeleteBuffers(1, &buffer->alBuffer);
        buffer->alBuffer = 0;
        return 0;
    }
    return buffer->alBuffer;
}

void LIBMATTI_MC_SoundBuffer_Free(LIBMATTI_MC_SoundBuffer *buffer)
{
    if (buffer == NULL)
        return;
    if (buffer->alBuffer != 0)
        LIBMATTI_OAL_AL_DeleteBuffers(1, &buffer->alBuffer);
    free(buffer);
}

// ---------------------------------------------------------------------------
// Channel
// ---------------------------------------------------------------------------

LIBMATTI_MC_SoundChannel *LIBMATTI_MC_SoundChannel_New(LIBMATTI_MC_SoundLibrary *library)
{
    LIBMATTI_MC_SoundChannel *channel = calloc(1, sizeof(LIBMATTI_MC_SoundChannel));
    if (channel == NULL)
        return NULL;
    channel->source = 0;
    channel->initialized = 0;
    channel->stopped = 0;
    if (library == NULL || !library->available)
        return channel; // the dead binding hands back the inert channel
    for (int i = 0; i < library->sourceCount; i++)
    {
        if (library->sources[i] != 0)
        {
            channel->source = library->sources[i];
            library->sources[i] = 0; // Java: the pool acquire takes the slot.
            channel->initialized = 1;
            break;
        }
    }
    return channel;
}

void LIBMATTI_MC_SoundChannel_Free(LIBMATTI_MC_SoundChannel *channel)
{
    if (channel == NULL)
        return;
    if (channel->initialized)
        LIBMATTI_OAL_AL_SourceStop(channel->source);
    free(channel);
}

void LIBMATTI_MC_SoundChannel_Play(LIBMATTI_MC_SoundChannel *channel)
{
    if (channel == NULL || !channel->initialized)
        return;
    LIBMATTI_OAL_AL_SourcePlay(channel->source);
}

void LIBMATTI_MC_SoundChannel_Pause(LIBMATTI_MC_SoundChannel *channel)
{
    if (channel == NULL || !channel->initialized)
        return;
    LIBMATTI_OAL_AL_SourcePause(channel->source);
}

void LIBMATTI_MC_SoundChannel_Unpause(LIBMATTI_MC_SoundChannel *channel)
{
    if (channel == NULL || !channel->initialized)
        return;
    // Java: unpause resumes the paused source.
    LIBMATTI_OAL_AL_SourcePlay(channel->source);
}

void LIBMATTI_MC_SoundChannel_Stop(LIBMATTI_MC_SoundChannel *channel)
{
    if (channel == NULL || !channel->initialized)
        return;
    LIBMATTI_OAL_AL_SourceStop(channel->source);
    channel->stopped = 1;
}

int LIBMATTI_MC_SoundChannel_Playing(const LIBMATTI_MC_SoundChannel *channel)
{
    if (channel == NULL || !channel->initialized)
        return 0;
    // Java: !this.initialized.get() ? false : AL_SOURCE_STATE == AL_PLAYING.
    return LIBMATTI_OAL_AL_GetSourcei(channel->source, LIBMATTI_OAL_AL_SOURCE_STATE)
           == LIBMATTI_OAL_AL_PLAYING;
}

int LIBMATTI_MC_SoundChannel_Stopped(const LIBMATTI_MC_SoundChannel *channel)
{
    if (channel == NULL)
        return 1;
    return channel->stopped
           || LIBMATTI_OAL_AL_GetSourcei(channel->source, LIBMATTI_OAL_AL_SOURCE_STATE)
                  == LIBMATTI_OAL_AL_STOPPED;
}

void LIBMATTI_MC_SoundChannel_SetSelfPosition(LIBMATTI_MC_SoundChannel *channel, float x, float y, float z)
{
    if (channel == NULL || !channel->initialized)
        return;
    // Java: alSourcefv(AL_POSITION, {x, y, z}) - the world-space position
    // (the relative flag switches the space, Java keeps the calls separate).
    LIBMATTI_OAL_AL_Source3f(channel->source, LIBMATTI_OAL_AL_POSITION, x, y, z);
}

void LIBMATTI_MC_SoundChannel_SetPitch(LIBMATTI_MC_SoundChannel *channel, float pitch)
{
    if (channel == NULL || !channel->initialized)
        return;
    LIBMATTI_OAL_AL_Sourcef(channel->source, LIBMATTI_OAL_AL_PITCH, pitch);
}

void LIBMATTI_MC_SoundChannel_SetLooping(LIBMATTI_MC_SoundChannel *channel, int looping)
{
    if (channel == NULL || !channel->initialized)
        return;
    LIBMATTI_OAL_AL_Sourcei(channel->source, LIBMATTI_OAL_AL_LOOPING, looping ? 1 : 0);
}

void LIBMATTI_MC_SoundChannel_SetRelative(LIBMATTI_MC_SoundChannel *channel, int relative)
{
    if (channel == NULL || !channel->initialized)
        return;
    LIBMATTI_OAL_AL_Sourcei(channel->source, LIBMATTI_OAL_AL_SOURCE_RELATIVE, relative ? 1 : 0);
}

void LIBMATTI_MC_SoundChannel_LinearAttenuation(LIBMATTI_MC_SoundChannel *channel, float distance)
{
    if (channel == NULL || !channel->initialized)
        return;
    // Java: linearAttenuation - AL_LINEAR_DISTANCE + the reference distance.
    LIBMATTI_OAL_AL_Sourcei(channel->source, LIBMATTI_OAL_AL_DISTANCE_MODEL,
                            LIBMATTI_OAL_AL_LINEAR_DISTANCE_CLAMPED);
    LIBMATTI_OAL_AL_Sourcef(channel->source, LIBMATTI_OAL_AL_REFERENCE_DISTANCE, distance);
    LIBMATTI_OAL_AL_Sourcef(channel->source, LIBMATTI_OAL_AL_MAX_DISTANCE, distance * 4.0f);
    LIBMATTI_OAL_AL_Sourcef(channel->source, LIBMATTI_OAL_AL_ROLLOFF_FACTOR, 1.0f);
}

void LIBMATTI_MC_SoundChannel_SetGain(LIBMATTI_MC_SoundChannel *channel, float gain)
{
    if (channel == NULL || !channel->initialized)
        return;
    LIBMATTI_OAL_AL_Sourcef(channel->source, LIBMATTI_OAL_AL_GAIN, gain);
}

void LIBMATTI_MC_SoundChannel_AttachStaticBuffer(LIBMATTI_MC_SoundChannel *channel,
                                                 LIBMATTI_MC_SoundBuffer *buffer)
{
    if (channel == NULL || !channel->initialized || buffer == NULL)
        return;
    unsigned int alBuffer = LIBMATTI_MC_SoundBuffer_Upload(buffer);
    if (alBuffer == 0)
        return;
    // Java: alSourcei(AL_BUFFER, buffer).
    LIBMATTI_OAL_AL_Sourcei(channel->source, LIBMATTI_OAL_AL_BUFFER, (int) alBuffer);
}

// ---------------------------------------------------------------------------
// Library + Listener
// ---------------------------------------------------------------------------

LIBMATTI_MC_SoundLibrary *LIBMATTI_MC_SoundLibrary_New(void)
{
    LIBMATTI_MC_SoundLibrary *library = calloc(1, sizeof(LIBMATTI_MC_SoundLibrary));
    if (library == NULL)
        return NULL;
    library->available = 0;

    // Java: Library.init - the device opens with the default name, the
    // context carries the default attributes (the HRTF flag rides later).
    if (!LIBMATTI_OAL_IsAvailable())
        return library;
    library->device = LIBMATTI_OAL_ALC_OpenDevice(NULL);
    if (library->device == 0)
        return library; // the CI machines have no audio device
    library->context = LIBMATTI_OAL_ALC_CreateContext(library->device, NULL);
    if (library->context == 0 || !LIBMATTI_OAL_ALC_MakeContextCurrent(library->context))
    {
        LIBMATTI_OAL_ALC_CloseDevice(library->device);
        library->device = 0;
        return library;
    }

    // Java: DEFAULT_CHANNEL_COUNT = 30 - the pool the engine acquires from.
    library->sourceCount = 30;
    LIBMATTI_OAL_AL_GenSources(library->sourceCount, library->sources);
    // Java: the linear-distance model is a hard requirement (the engine
    // throws without it); the port keeps the flag and the channels fall back
    // to the default model when the extension is missing.
    LIBMATTI_OAL_AL_DistanceModel(LIBMATTI_OAL_AL_LINEAR_DISTANCE_CLAMPED);
    library->available = 1;
    return library;
}

void LIBMATTI_MC_SoundLibrary_SetListener(LIBMATTI_MC_SoundLibrary *library,
                                          const LIBMATTI_MC_ListenerTransform *transform)
{
    if (library == NULL || !library->available || transform == NULL)
        return;
    // Java: Listener.setTransform - alListener3f(AL_POSITION) +
    // alListenerfv(AL_ORIENTATION, {forward, up}).
    LIBMATTI_OAL_AL_Listener3f(LIBMATTI_OAL_AL_POSITION,
                               transform->position[0], transform->position[1],
                               transform->position[2]);
    float orientation[6] = {
        transform->forward[0], transform->forward[1], transform->forward[2],
        transform->up[0], transform->up[1], transform->up[2],
    };
    LIBMATTI_OAL_AL_Listenerfv(LIBMATTI_OAL_AL_ORIENTATION, orientation);
}

int LIBMATTI_MC_SoundLibrary_HasLinearDistance(const LIBMATTI_MC_SoundLibrary *library)
{
    if (library == NULL || !library->available)
        return 0;
    return LIBMATTI_OAL_HasExtLinearDistance();
}

void LIBMATTI_MC_SoundLibrary_Close(LIBMATTI_MC_SoundLibrary *library)
{
    if (library == NULL)
        return;
    if (library->available)
    {
        // Java: the pool cleanup stops + deletes every live source.
        LIBMATTI_OAL_AL_DeleteSources(library->sourceCount, library->sources);
        LIBMATTI_OAL_ALC_MakeContextCurrent(0);
        if (library->context != 0)
            LIBMATTI_OAL_ALC_DestroyContext(library->context);
        if (library->device != 0)
            LIBMATTI_OAL_ALC_CloseDevice(library->device);
        library->available = 0;
    }
}

// Java: Library.close - the full teardown + the struct release.
void LIBMATTI_MC_SoundLibrary_Free(LIBMATTI_MC_SoundLibrary *library)
{
    if (library == NULL)
        return;
    LIBMATTI_MC_SoundLibrary_Close(library);
    free(library);
}

// ---------------------------------------------------------------------------
// SoundEvents (the synthesized stand-in for the resource-pack sounds)
// ---------------------------------------------------------------------------

typedef struct SoundEntry
{
    char *id;
    LIBMATTI_MC_SoundBuffer *buffer;
    struct SoundEntry *next;
} SoundEntry;

static SoundEntry *g_soundEntries = NULL;
static int g_soundEntryCount = 0;

// Java: SoundEvents registers ~1500 constants; the demo resolves the tones
// the SoundTypes reference (one entry per resolved id, lazily synthesized).
static const struct
{
    const char *suffix; // the id part after "block.<family>."
    float frequency;    // the synthesis tone (the family's "pitch" identity)
    float duration;
} SOUND_TONES[] = {
    {"break", 180.0f, 0.18f},
    {"step", 220.0f, 0.09f},
    {"place", 200.0f, 0.14f},
    {"hit", 260.0f, 0.07f},
    {"fall", 150.0f, 0.12f},
    {NULL, 0.0f, 0.0f},
};

static LIBMATTI_MC_SoundBuffer *synthesize_tone(float frequency, float duration)
{
    int sampleRate = 22050;
    int sampleCount = (int) (duration * sampleRate);
    short *data = malloc((size_t) sampleCount * sizeof(short));
    if (data == NULL)
        return NULL;
    for (int i = 0; i < sampleCount; i++)
    {
        float t = (float) i / (float) sampleRate;
        // the decay envelope (the click-free tail)
        float envelope = 1.0f - (t / duration);
        data[i] = (short) (9000.0f * envelope * sinf(2.0f * 3.14159265f * frequency * t));
    }
    return LIBMATTI_MC_SoundBuffer_New(data, sampleCount, 1, sampleRate);
}

LIBMATTI_MC_SoundBuffer *LIBMATTI_MC_SoundEvents_Resolve(const char *soundEventId)
{
    if (soundEventId == NULL)
        return NULL;
    // Java: the registry dedupes by id - the cache walks the entry list.
    for (SoundEntry *entry = g_soundEntries; entry != NULL; entry = entry->next)
    {
        if (strcmp(entry->id, soundEventId) == 0)
            return entry->buffer;
    }

    // the tone pick: the id's action suffix ("...break" etc.) selects the
    // synthesis voice; unknown ids ride the "place" default.
    float frequency = 200.0f, duration = 0.14f;
    for (int i = 0; SOUND_TONES[i].suffix != NULL; i++)
    {
        size_t suffixLength = strlen(SOUND_TONES[i].suffix);
        size_t idLength = strlen(soundEventId);
        if (idLength > suffixLength
            && strcmp(soundEventId + idLength - suffixLength, SOUND_TONES[i].suffix) == 0)
        {
            frequency = SOUND_TONES[i].frequency;
            duration = SOUND_TONES[i].duration;
            break;
        }
    }

    LIBMATTI_MC_SoundBuffer *buffer = synthesize_tone(frequency, duration);
    SoundEntry *entry = calloc(1, sizeof(SoundEntry));
    if (entry == NULL)
        return buffer;
    entry->id = strdup(soundEventId);
    entry->buffer = buffer;
    entry->next = g_soundEntries;
    g_soundEntries = entry;
    if (buffer != NULL)
        g_soundEntryCount++;
    return buffer;
}

int LIBMATTI_MC_SoundEvents_Count(void)
{
    return g_soundEntryCount;
}

// ---------------------------------------------------------------------------
// SoundEngine
// ---------------------------------------------------------------------------

LIBMATTI_MC_SoundEngine *LIBMATTI_MC_SoundEngine_New(void)
{
    LIBMATTI_MC_SoundEngine *engine = calloc(1, sizeof(LIBMATTI_MC_SoundEngine));
    if (engine == NULL)
        return NULL;
    engine->masterVolume = 1.0f;
    engine->library = NULL; // Java: the boot rides the first reload.
    return engine;
}

void LIBMATTI_MC_SoundEngine_Free(LIBMATTI_MC_SoundEngine *engine)
{
    if (engine == NULL)
        return;
    if (engine->library != NULL)
        LIBMATTI_MC_SoundLibrary_Free(engine->library);
    free(engine);
}

void LIBMATTI_MC_SoundEngine_Reload(LIBMATTI_MC_SoundEngine *engine)
{
    if (engine == NULL)
        return;
    if (engine->library == NULL)
        engine->library = LIBMATTI_MC_SoundLibrary_New();
}

void LIBMATTI_MC_SoundEngine_Tick(LIBMATTI_MC_SoundEngine *engine,
                                  const LIBMATTI_MC_ListenerTransform *transform)
{
    if (engine == NULL || engine->library == NULL)
        return;
    LIBMATTI_MC_SoundLibrary_SetListener(engine->library, transform);
}

void LIBMATTI_MC_SoundEngine_Play(LIBMATTI_MC_SoundEngine *engine, const char *soundEventId,
                                  float x, float y, float z, float volume, float pitch)
{
    if (engine == NULL || soundEventId == NULL)
        return;
    if (engine->library == NULL)
        LIBMATTI_MC_SoundEngine_Reload(engine);
    LIBMATTI_MC_SoundLibrary *library = engine->library;
    if (library == NULL || !library->available)
        return; // the dead audio path stays silent (headless CI)

    LIBMATTI_MC_SoundBuffer *buffer = LIBMATTI_MC_SoundEvents_Resolve(soundEventId);
    if (buffer == NULL)
        return;
    LIBMATTI_MC_SoundChannel *channel = LIBMATTI_MC_SoundChannel_New(library);
    if (channel == NULL || !channel->initialized)
    {
        LIBMATTI_MC_SoundChannel_Free(channel);
        return;
    }
    // Java: the play path folds the SoundType's volume/pitch over the master
    // volume, the linear attenuation rides the reference distance 16.
    LIBMATTI_MC_SoundChannel_AttachStaticBuffer(channel, buffer);
    LIBMATTI_MC_SoundChannel_SetGain(channel, volume * engine->masterVolume);
    LIBMATTI_MC_SoundChannel_SetPitch(channel, pitch);
    LIBMATTI_MC_SoundChannel_SetSelfPosition(channel, x, y, z);
    LIBMATTI_MC_SoundChannel_SetRelative(channel, 0);
    LIBMATTI_MC_SoundChannel_LinearAttenuation(channel, 16.0f);
    LIBMATTI_MC_SoundChannel_Play(channel);
    // Java: the ChannelHandle detaches on the AL_STOPPED transition - the
    // port frees the channel here (the static buffer keeps the PCM alive in
    // the SoundEvents cache; the source returns to the pool on Close).
    LIBMATTI_MC_SoundChannel_Free(channel);
}

void LIBMATTI_MC_SoundEngine_PlayUi(LIBMATTI_MC_SoundEngine *engine, const char *soundEventId,
                                    float volume, float pitch)
{
    // Java: SimpleSoundInstance.forUI - the RELATIVE source at the listener
    // origin (the 2D path).
    if (engine == NULL)
        return;
    if (engine->library == NULL)
        LIBMATTI_MC_SoundEngine_Reload(engine);
    LIBMATTI_MC_SoundLibrary *library = engine->library;
    if (library == NULL || !library->available)
        return;
    LIBMATTI_MC_SoundBuffer *buffer = LIBMATTI_MC_SoundEvents_Resolve(soundEventId);
    if (buffer == NULL)
        return;
    LIBMATTI_MC_SoundChannel *channel = LIBMATTI_MC_SoundChannel_New(library);
    if (channel == NULL || !channel->initialized)
    {
        LIBMATTI_MC_SoundChannel_Free(channel);
        return;
    }
    LIBMATTI_MC_SoundChannel_AttachStaticBuffer(channel, buffer);
    LIBMATTI_MC_SoundChannel_SetGain(channel, volume * engine->masterVolume);
    LIBMATTI_MC_SoundChannel_SetPitch(channel, pitch);
    LIBMATTI_MC_SoundChannel_SetSelfPosition(channel, 0.0f, 0.0f, 0.0f);
    LIBMATTI_MC_SoundChannel_SetRelative(channel, 1);
    LIBMATTI_MC_SoundChannel_Play(channel);
    LIBMATTI_MC_SoundChannel_Free(channel);
}

void LIBMATTI_MC_SoundEngine_StopAll(LIBMATTI_MC_SoundEngine *engine)
{
    if (engine == NULL || engine->library == NULL || !engine->library->available)
        return;
    for (int i = 0; i < engine->library->sourceCount; i++)
        if (engine->library->sources[i] != 0)
            LIBMATTI_OAL_AL_SourceStop(engine->library->sources[i]);
}

void LIBMATTI_MC_SoundEngine_Pause(LIBMATTI_MC_SoundEngine *engine)
{
    if (engine == NULL || engine->library == NULL || !engine->library->available)
        return;
    for (int i = 0; i < engine->library->sourceCount; i++)
        if (engine->library->sources[i] != 0)
            LIBMATTI_OAL_AL_SourcePause(engine->library->sources[i]);
}

void LIBMATTI_MC_SoundEngine_Resume(LIBMATTI_MC_SoundEngine *engine)
{
    if (engine == NULL || engine->library == NULL || !engine->library->available)
        return;
    for (int i = 0; i < engine->library->sourceCount; i++)
        if (engine->library->sources[i] != 0)
            LIBMATTI_OAL_AL_SourcePlay(engine->library->sources[i]);
}

void LIBMATTI_MC_SoundEngine_SetVolume(LIBMATTI_MC_SoundEngine *engine, float volume)
{
    if (engine == NULL)
        return;
    // Java: the options' master volume clamps to the 0..1 slider range.
    engine->masterVolume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
}

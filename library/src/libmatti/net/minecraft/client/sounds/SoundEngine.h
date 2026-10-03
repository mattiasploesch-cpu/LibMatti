// Port of the Minecraft client sound slice over the LIBMATTI_OAL_* wrapper:
// blaze3d.audio (Channel, Listener, SoundBuffer, Library) + the SoundEngine /
// SoundManager front (net.minecraft.client.sounds). The wrapper is the only
// OpenAL touchpoint - the engine drives the AL objects through it.

#ifndef MATTICRAFT_MC_CLIENT_SOUNDS_SOUNDENGINE_H
#define MATTICRAFT_MC_CLIENT_SOUNDS_SOUNDENGINE_H

#include "libmatti/net/minecraft/world/level/block/SoundType.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: com.mojang.blaze3d.audio.Channel - one AL source the engine plays.
typedef struct LIBMATTI_MC_SoundChannel
{
    // Java: private int pointer (the AL source handle); 0 = the dead binding.
    unsigned int source;
    // Java: private AtomicBoolean initialized - the allocation flag.
    int initialized;
    // Java: the stopping flag ChannelAccess sets (the detached channel drain).
    int stopped;
} LIBMATTI_MC_SoundChannel;

// Java: com.mojang.blaze3d.audio.SoundBuffer - the decoded PCM + the format
// the AL buffer wraps (the port carries the 16-bit mono/stereo parameters).
typedef struct LIBMATTI_MC_SoundBuffer
{
    // Java: private final ByteBuffer data
    short *data;
    int sampleCount;
    // Java: private final AudioFormat format - the port's channel count +
    // sample rate (16-bit PCM fixed, like the decoded vanilla sounds).
    int channels; // 1 = mono, 2 = stereo
    int sampleRate;
    // the AL buffer handle (0 until Upload runs on the live context)
    unsigned int alBuffer;
} LIBMATTI_MC_SoundBuffer;

// Java: com.mojang.blaze3d.audio.ListenerTransform - the position/forward/up
// triple the engine sets from the camera.
typedef struct LIBMATTI_MC_ListenerTransform
{
    float position[3];
    float forward[3];
    float up[3];
} LIBMATTI_MC_ListenerTransform;

// Java: com.mojang.blaze3d.audio.Library - the device/context boot + the
// channel pool (the static pool the engine acquires from).
typedef struct LIBMATTI_MC_SoundLibrary
{
    long device;
    long context;
    // Java: DEFAULT_CHANNEL_COUNT = 30
    unsigned int sources[30];
    int sourceCount;
    int available; // the OAL binding probe result
} LIBMATTI_MC_SoundLibrary;

// Java: net.minecraft.client.sounds.SoundEngine - the engine over the library.
typedef struct LIBMATTI_MC_SoundEngine
{
    LIBMATTI_MC_SoundLibrary *library;
    // Java: the sound engine keeps the global volume the options drive.
    float masterVolume;
} LIBMATTI_MC_SoundEngine;

// ---------------------------------------------------------------------------
// SoundBuffer (Java: SoundBuffer)
// ---------------------------------------------------------------------------

// Java: new SoundBuffer(data, format) - the port wraps the caller's samples
// (16-bit PCM; mono or stereo), the Upload owns the AL buffer afterwards.
LIBMATTI_MC_SoundBuffer *LIBMATTI_MC_SoundBuffer_New(short *data, int sampleCount,
                                                      int channels, int sampleRate);
// Java: SoundBuffer.getAlBuffer() - the upload path (alGenBuffers +
// alBufferData over the OAL wrapper); 0 when the binding is dead.
unsigned int LIBMATTI_MC_SoundBuffer_Upload(LIBMATTI_MC_SoundBuffer *buffer);
// Java: SoundBuffer.discardAlBuffer - the delete path.
void LIBMATTI_MC_SoundBuffer_Free(LIBMATTI_MC_SoundBuffer *buffer);

// ---------------------------------------------------------------------------
// Channel (Java: Channel)
// ---------------------------------------------------------------------------

// Java: new Channel(Library, staticPool) - the AL source allocation rides the
// library's pool; a dead binding hands back the uninitialized channel.
LIBMATTI_MC_SoundChannel *LIBMATTI_MC_SoundChannel_New(LIBMATTI_MC_SoundLibrary *library);
void LIBMATTI_MC_SoundChannel_Free(LIBMATTI_MC_SoundChannel *channel);
// Java: Channel.play / pause / unpause / stop
void LIBMATTI_MC_SoundChannel_Play(LIBMATTI_MC_SoundChannel *channel);
void LIBMATTI_MC_SoundChannel_Pause(LIBMATTI_MC_SoundChannel *channel);
void LIBMATTI_MC_SoundChannel_Unpause(LIBMATTI_MC_SoundChannel *channel);
void LIBMATTI_MC_SoundChannel_Stop(LIBMATTI_MC_SoundChannel *channel);
// Java: Channel.playing() / stopped()
int LIBMATTI_MC_SoundChannel_Playing(const LIBMATTI_MC_SoundChannel *channel);
int LIBMATTI_MC_SoundChannel_Stopped(const LIBMATTI_MC_SoundChannel *channel);
// Java: Channel.setSelfPosition(Vec3)
void LIBMATTI_MC_SoundChannel_SetSelfPosition(LIBMATTI_MC_SoundChannel *channel, float x, float y, float z);
// Java: Channel.setPitch / setLooping / setRelative / linearAttenuation
void LIBMATTI_MC_SoundChannel_SetPitch(LIBMATTI_MC_SoundChannel *channel, float pitch);
void LIBMATTI_MC_SoundChannel_SetLooping(LIBMATTI_MC_SoundChannel *channel, int looping);
void LIBMATTI_MC_SoundChannel_SetRelative(LIBMATTI_MC_SoundChannel *channel, int relative);
void LIBMATTI_MC_SoundChannel_LinearAttenuation(LIBMATTI_MC_SoundChannel *channel, float distance);
// Java: Channel.setGain(float) (the volume the engine folds)
void LIBMATTI_MC_SoundChannel_SetGain(LIBMATTI_MC_SoundChannel *channel, float gain);
// Java: Channel.attachStaticBuffer(SoundBuffer)
void LIBMATTI_MC_SoundChannel_AttachStaticBuffer(LIBMATTI_MC_SoundChannel *channel,
                                                 LIBMATTI_MC_SoundBuffer *buffer);

// ---------------------------------------------------------------------------
// Library (Java: Library) + Listener
// ---------------------------------------------------------------------------

// Java: Library.init - the device/context boot + the channel pool allocation;
// a dead binding or a missing device leaves available = 0 (the engine no-ops).
LIBMATTI_MC_SoundLibrary *LIBMATTI_MC_SoundLibrary_New(void);
void LIBMATTI_MC_SoundLibrary_Free(LIBMATTI_MC_SoundLibrary *library);
// Java: Library.getListener().setTransform - the camera coupling.
void LIBMATTI_MC_SoundLibrary_SetListener(LIBMATTI_MC_SoundLibrary *library,
                                          const LIBMATTI_MC_ListenerTransform *transform);
// Java: Library.loadSoundCapabilites - the linear-distance requirement.
int LIBMATTI_MC_SoundLibrary_HasLinearDistance(const LIBMATTI_MC_SoundLibrary *library);
// Java: Library.close - the full teardown.
void LIBMATTI_MC_SoundLibrary_Close(LIBMATTI_MC_SoundLibrary *library);

// ---------------------------------------------------------------------------
// SoundEngine (Java: SoundEngine)
// ---------------------------------------------------------------------------

// Java: SoundEngine(options) - the boot is lazy like Java's (the engine
// loads on the first reload, the client ticks it).
LIBMATTI_MC_SoundEngine *LIBMATTI_MC_SoundEngine_New(void);
void LIBMATTI_MC_SoundEngine_Free(LIBMATTI_MC_SoundEngine *engine);
// Java: SoundEngine.reload - the library boot + the sound event registration.
void LIBMATTI_MC_SoundEngine_Reload(LIBMATTI_MC_SoundEngine *engine);
// Java: SoundEngine.tick - the listener rides the camera every tick.
void LIBMATTI_MC_SoundEngine_Tick(LIBMATTI_MC_SoundEngine *engine, const LIBMATTI_MC_ListenerTransform *transform);
// Java: SoundEngine.play(LocalPlayer, SoundInstance) - the world-space play
// (the position comes with the call; the volume/pitch scale the SoundType).
void LIBMATTI_MC_SoundEngine_Play(LIBMATTI_MC_SoundEngine *engine, const char *soundEventId,
                                  float x, float y, float z, float volume, float pitch);
// Java: the UI play (SimpleSoundInstance.forUI) - the relative 2D path.
void LIBMATTI_MC_SoundEngine_PlayUi(LIBMATTI_MC_SoundEngine *engine, const char *soundEventId,
                                    float volume, float pitch);
// Java: SoundEngine.stopAll / pause / resume
void LIBMATTI_MC_SoundEngine_StopAll(LIBMATTI_MC_SoundEngine *engine);
void LIBMATTI_MC_SoundEngine_Pause(LIBMATTI_MC_SoundEngine *engine);
void LIBMATTI_MC_SoundEngine_Resume(LIBMATTI_MC_SoundEngine *engine);
// Java: SoundEngine.setVolume - the options' master volume.
void LIBMATTI_MC_SoundEngine_SetVolume(LIBMATTI_MC_SoundEngine *engine, float volume);

// ---------------------------------------------------------------------------
// SoundEvents (Java: net.minecraft.sounds.SoundEvents) - the constant names
// the SoundTypes carry ("minecraft:block.stone.break" style), resolved to a
// synthesized PCM buffer (the port's stand-in for the resource-pack oggs:
// the engine synthesizes the tone so the pipeline is testable without the
// assets; the resource pack's sounds ride when the decoder lands).
// ---------------------------------------------------------------------------

// The lookup: "block.stone.break" -> the synthesized buffer (cached per id).
LIBMATTI_MC_SoundBuffer *LIBMATTI_MC_SoundEvents_Resolve(const char *soundEventId);
// The registry size (the harness checks).
int LIBMATTI_MC_SoundEvents_Count(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_SOUNDS_SOUNDENGINE_H

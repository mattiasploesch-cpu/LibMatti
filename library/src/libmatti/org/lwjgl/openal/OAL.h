// The OpenAL wrapper (see OAL.h) - everything the sound engine needs from
// OpenAL behind the LIBMATTI_OAL_* surface. The binding resolves the native
// libopenal at runtime with dlopen (a missing library means "unavailable"
// instead of a hard link error, the GLFW/GL pattern).

#ifndef MATTICRAFT_LIBMATTI_ORG_LWJGL_OPENAL_OAL_H
#define MATTICRAFT_LIBMATTI_ORG_LWJGL_OPENAL_OAL_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: org.lwjgl.openal.ALC10/AL10 - the constants the sound engine touches.
// (The raw AL enum values; the wrapper keeps them so the engine never sees
// the real headers.)
#define LIBMATTI_OAL_ALC_FALSE 0
#define LIBMATTI_OAL_ALC_TRUE 1
#define LIBMATTI_OAL_ALC_FREQUENCY 0x1007
#define LIBMATTI_OAL_ALC_REFRESH 0x1008
#define LIBMATTI_OAL_ALC_SYNC 0x1009
#define LIBMATTI_OAL_ALC_MONO_SOURCES 0x1010
#define LIBMATTI_OAL_ALC_STEREO_SOURCES 0x1011
#define LIBMATTI_OAL_ALC_INVALID_DEVICE 0xA001
#define LIBMATTI_OAL_ALC_INVALID_CONTEXT 0xA002
#define LIBMATTI_OAL_ALC_INVALID_ENUM 0xA003
#define LIBMATTI_OAL_ALC_INVALID_VALUE 0xA004
#define LIBMATTI_OAL_ALC_OUT_OF_MEMORY 0xA005
#define LIBMATTI_OAL_ALC_DEVICE_SPECIFIER 0x1004
#define LIBMATTI_OAL_ALC_DEFAULT_DEVICE_SPECIFIER 0x1004
#define LIBMATTI_OAL_ALC_EXTENSIONS 0x1006

#define LIBMATTI_OAL_AL_NONE 0
#define LIBMATTI_OAL_AL_FALSE 0
#define LIBMATTI_OAL_AL_TRUE 1
#define LIBMATTI_OAL_AL_SOURCE_RELATIVE 0x202
#define LIBMATTI_OAL_AL_CONE_INNER_ANGLE 0x1001
#define LIBMATTI_OAL_AL_CONE_OUTER_ANGLE 0x1002
#define LIBMATTI_OAL_AL_PITCH 0x1003
#define LIBMATTI_OAL_AL_POSITION 0x1004
#define LIBMATTI_OAL_AL_DIRECTION 0x1005
#define LIBMATTI_OAL_AL_VELOCITY 0x1006
#define LIBMATTI_OAL_AL_LOOPING 0x1007
#define LIBMATTI_OAL_AL_BUFFER 0x1009
#define LIBMATTI_OAL_AL_GAIN 0x100A
#define LIBMATTI_OAL_AL_MIN_GAIN 0x100D
#define LIBMATTI_OAL_AL_MAX_GAIN 0x100E
#define LIBMATTI_OAL_AL_ORIENTATION 0x100F
#define LIBMATTI_OAL_AL_SOURCE_STATE 0x1010
#define LIBMATTI_OAL_AL_INITIAL 0x1011
#define LIBMATTI_OAL_AL_PLAYING 0x1012
#define LIBMATTI_OAL_AL_PAUSED 0x1013
#define LIBMATTI_OAL_AL_STOPPED 0x1014
#define LIBMATTI_OAL_AL_BUFFERS_QUEUED 0x1015
#define LIBMATTI_OAL_AL_BUFFERS_PROCESSED 0x1016
#define LIBMATTI_OAL_AL_REFERENCE_DISTANCE 0x1020
#define LIBMATTI_OAL_AL_ROLLOFF_FACTOR 0x1021
#define LIBMATTI_OAL_AL_CONE_OUTER_GAIN 0x1022
#define LIBMATTI_OAL_AL_MAX_DISTANCE 0x1023
#define LIBMATTI_OAL_AL_DISTANCE_MODEL 0xC000
#define LIBMATTI_OAL_AL_INVERSE_DISTANCE 0xD001
#define LIBMATTI_OAL_AL_INVERSE_DISTANCE_CLAMPED 0xD002
#define LIBMATTI_OAL_AL_LINEAR_DISTANCE 0xD003
#define LIBMATTI_OAL_AL_LINEAR_DISTANCE_CLAMPED 0xD004
#define LIBMATTI_OAL_AL_NO_ERROR 0
#define LIBMATTI_OAL_AL_INVALID_NAME 0xA001
#define LIBMATTI_OAL_AL_INVALID_ENUM 0xA002
#define LIBMATTI_OAL_AL_INVALID_VALUE 0xA003
#define LIBMATTI_OAL_AL_INVALID_OPERATION 0xA004
#define LIBMATTI_OAL_AL_OUT_OF_MEMORY 0xA005
#define LIBMATTI_OAL_AL_VENDOR 0xB001
#define LIBMATTI_OAL_AL_VERSION 0xB002
#define LIBMATTI_OAL_AL_RENDERER 0xB003
#define LIBMATTI_OAL_AL_EXTENSIONS 0xB004
#define LIBMATTI_OAL_AL_FORMAT_MONO8 0x1100
#define LIBMATTI_OAL_AL_FORMAT_MONO16 0x1101
#define LIBMATTI_OAL_AL_FORMAT_STEREO8 0x1102
#define LIBMATTI_OAL_AL_FORMAT_STEREO16 0x1103

// ---------------------------------------------------------------------------
// ALC (Java: ALC10 - the device/context layer)
// ---------------------------------------------------------------------------

// Java: ALC10.alcOpenDevice(String) - NULL opens the default device.
long LIBMATTI_OAL_ALC_OpenDevice(const char *deviceName);
// Java: ALC10.alcCloseDevice
int LIBMATTI_OAL_ALC_CloseDevice(long device);
// Java: ALC10.alcCreateContext - the attrs array is ALCenum/value pairs,
// zero-terminated (NULL = defaults).
long LIBMATTI_OAL_ALC_CreateContext(long device, const int *attrs);
// Java: ALC10.alcMakeContextCurrent
int LIBMATTI_OAL_ALC_MakeContextCurrent(long context);
// Java: ALC10.alcDestroyContext
void LIBMATTI_OAL_ALC_DestroyContext(long context);
// Java: ALC10.alcGetError
int LIBMATTI_OAL_ALC_GetError(long device);
// Java: ALC10.alcGetString - the returned string stays owned by AL.
const char *LIBMATTI_OAL_ALC_GetString(long device, int param);
// Java: ALC10.alcIsExtensionPresent
int LIBMATTI_OAL_ALC_IsExtensionPresent(long device, const char *extension);
// Java: ALC11.alcGetString(device, ALC_ALL_DEVICES_SPECIFIER) etc. ride the
// same call (the param selects).

// ---------------------------------------------------------------------------
// AL (Java: AL10 - the rendering layer)
// ---------------------------------------------------------------------------

// Java: AL10.alGenBuffers(IntBuffer) / alGenSources(IntBuffer) - the count
// variant fills the caller's array; 0 fills mean allocation failure.
void LIBMATTI_OAL_AL_GenBuffers(int count, unsigned int *buffers);
void LIBMATTI_OAL_AL_GenSources(int count, unsigned int *sources);
void LIBMATTI_OAL_AL_DeleteBuffers(int count, const unsigned int *buffers);
void LIBMATTI_OAL_AL_DeleteSources(int count, const unsigned int *sources);
// Java: AL10.alBufferData(buffer, format, data, sampleRate)
void LIBMATTI_OAL_AL_BufferData(unsigned int buffer, int format, const void *data,
                                 int sizeBytes, int sampleRate);
// Java: AL10.alDeleteBuffers etc. ride above.

// Java: AL10.alSourcePlay/Stop/Pause/Rewind (single-source forms)
void LIBMATTI_OAL_AL_SourcePlay(unsigned int source);
void LIBMATTI_OAL_AL_SourceStop(unsigned int source);
void LIBMATTI_OAL_AL_SourcePause(unsigned int source);
// Java: AL10.alSourcei/alSourcef/alSource3f/alSourcefv
void LIBMATTI_OAL_AL_Sourcei(unsigned int source, int param, int value);
void LIBMATTI_OAL_AL_Sourcef(unsigned int source, int param, float value);
void LIBMATTI_OAL_AL_Source3f(unsigned int source, int param, float x, float y, float z);
void LIBMATTI_OAL_AL_Sourcefv(unsigned int source, int param, const float *values);
// Java: AL10.alGetSourcei
int LIBMATTI_OAL_AL_GetSourcei(unsigned int source, int param);
// Java: AL10.alSourceQueueBuffers / alSourceUnqueueBuffers (the streaming path)
void LIBMATTI_OAL_AL_SourceQueueBuffers(unsigned int source, int count, const unsigned int *buffers);
void LIBMATTI_OAL_AL_SourceUnqueueBuffers(unsigned int source, int count, unsigned int *buffers);

// Java: AL10.alListener3f/alListenerfv
void LIBMATTI_OAL_AL_Listener3f(int param, float x, float y, float z);
void LIBMATTI_OAL_AL_Listenerfv(int param, const float *values);
void LIBMATTI_OAL_AL_Listenerf(int param, float value);
// Java: AL10.alDistanceModel - the engine sets LINEAR_DISTANCE_CLAMPED.
void LIBMATTI_OAL_AL_DistanceModel(int model);
// Java: AL10.alDopplerFactor
void LIBMATTI_OAL_AL_DopplerFactor(float value);
// Java: AL10.alGetError
int LIBMATTI_OAL_AL_GetError(void);
// Java: AL10.alGetString
const char *LIBMATTI_OAL_AL_GetString(int param);
// Java: AL10.alIsSource / alIsBuffer
int LIBMATTI_OAL_AL_IsSource(unsigned int source);
int LIBMATTI_OAL_AL_IsBuffer(unsigned int buffer);

// ---------------------------------------------------------------------------
// capability surface (Java: ALCapabilities/ALCCapabilities - the flags the
// Library.init path checks)
// ---------------------------------------------------------------------------

int LIBMATTI_OAL_IsAvailable(void);
int LIBMATTI_OAL_HasExtLinearDistance(void);
// Java: ALUtil.getStringList - the device enumeration (NULL-terminated array
// the caller frees; NULL when the extension is missing).
char **LIBMATTI_OAL_ALC_GetAllDeviceSpecifiers(void);
void LIBMATTI_OAL_ALC_FreeDeviceSpecifiers(char **specifiers);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_LIBMATTI_ORG_LWJGL_OPENAL_OAL_H

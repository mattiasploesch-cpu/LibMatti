// The OpenAL wrapper (see OAL.h) - everything the sound engine needs from
// OpenAL behind the LIBMATTI_OAL_* surface. The binding resolves the native
// libopenal at runtime with dlopen, exactly like the tinyfd binding does for
// tinyfiledialogs. A missing or unloaded library means "not available"
// instead of a hard link error (the headless test machines keep booting).

#include "libmatti/org/lwjgl/openal/OAL.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *oal_lib = NULL;

#define OAL_DLSYM(field, name)                                                                   \
    do                                                                                           \
    {                                                                                            \
        if (oal_##field == NULL) oal_##field = dlsym(oal_lib, name);                             \
    } while (0)

// ---------------------------------------------------------------------------
// ALC function pointers (Java: ALC10/ALC11)
// ---------------------------------------------------------------------------
static long (*oal_alcOpenDevice)(const char *) = NULL;
static int (*oal_alcCloseDevice)(long) = NULL;
static long (*oal_alcCreateContext)(long, const int *) = NULL;
static int (*oal_alcMakeContextCurrent)(long) = NULL;
static void (*oal_alcDestroyContext)(long) = NULL;
static int (*oal_alcGetError)(long) = NULL;
static const char *(*oal_alcGetString)(long, int) = NULL;
static int (*oal_alcIsExtensionPresent)(long, const char *) = NULL;
static const char *(*oal_alcGetStringPtr)(long, int) = NULL;

// ---------------------------------------------------------------------------
// AL function pointers (Java: AL10)
// ---------------------------------------------------------------------------
static void (*oal_alGenBuffers)(int, unsigned int *) = NULL;
static void (*oal_alGenSources)(int, unsigned int *) = NULL;
static void (*oal_alDeleteBuffers)(int, const unsigned int *) = NULL;
static void (*oal_alDeleteSources)(int, const unsigned int *) = NULL;
static void (*oal_alBufferData)(unsigned int, int, const void *, int, int) = NULL;
static void (*oal_alSourcePlay)(unsigned int) = NULL;
static void (*oal_alSourceStop)(unsigned int) = NULL;
static void (*oal_alSourcePause)(unsigned int) = NULL;
static void (*oal_alSourcei)(unsigned int, int, int) = NULL;
static void (*oal_alSourcef)(unsigned int, int, float) = NULL;
static void (*oal_alSource3f)(unsigned int, int, float, float, float) = NULL;
static void (*oal_alSourcefv)(unsigned int, int, const float *) = NULL;
static void (*oal_alGetSourcei)(unsigned int, int, int *) = NULL;
static void (*oal_alSourceQueueBuffers)(unsigned int, int, const unsigned int *) = NULL;
static void (*oal_alSourceUnqueueBuffers)(unsigned int, int, unsigned int *) = NULL;
static void (*oal_alListener3f)(int, float, float, float) = NULL;
static void (*oal_alListenerfv)(int, const float *) = NULL;
static void (*oal_alListenerf)(int, float) = NULL;
static void (*oal_alDistanceModel)(int) = NULL;
static void (*oal_alDopplerFactor)(float) = NULL;
static int (*oal_alGetError)(void) = NULL;
static const char *(*oal_alGetString)(int) = NULL;
static int (*oal_alIsSource)(unsigned int) = NULL;
static int (*oal_alIsBuffer)(unsigned int) = NULL;

// Java: AL.createCapabilities/ALC.createCapabilities resolve the function
// table once - the port's dlopen equivalents ride the same one-time probe.
static int oal_available = -1; // -1 = unprobed

static void oal_probe(void)
{
    if (oal_available >= 0)
        return;
    // Java: the LWJGL natives ship OpenAL Soft - the Linux names the loader
    // finds: the soname first, then the unversioned dev symlink.
    const char *names[] = {"libopenal.so.1", "libopenal.so", "libopenal.so.1.24.3", NULL};
    for (int i = 0; names[i] != NULL && oal_lib == NULL; i++)
    {
        oal_lib = dlopen(names[i], RTLD_NOW | RTLD_GLOBAL);
    }
    if (oal_lib == NULL)
    {
        oal_available = 0;
        return;
    }
    oal_available = 1;
}

int LIBMATTI_OAL_IsAvailable(void)
{
    oal_probe();
    return oal_available;
}

// ---------------------------------------------------------------------------
// ALC
// ---------------------------------------------------------------------------

long LIBMATTI_OAL_ALC_OpenDevice(const char *deviceName)
{
    oal_probe();
    if (!oal_available)
        return 0;
    // The DLSYM guard is the availability itself (the field IS the resolved
    // pointer) - the extra NULL pre-check short-circuited with the pointer
    // still NULL and every device open returned 0 (the dead-binding path).
    OAL_DLSYM(alcOpenDevice, "alcOpenDevice");
    if (oal_alcOpenDevice == NULL)
        return 0;
    return oal_alcOpenDevice(deviceName);
}

int LIBMATTI_OAL_ALC_CloseDevice(long device)
{
    oal_probe();
    if (!oal_available)
        return 0;
    OAL_DLSYM(alcCloseDevice, "alcCloseDevice");
    if (oal_alcCloseDevice == NULL)
        return 0;
    return oal_alcCloseDevice(device);
}

long LIBMATTI_OAL_ALC_CreateContext(long device, const int *attrs)
{
    oal_probe();
    if (!oal_available)
        return 0;
    OAL_DLSYM(alcCreateContext, "alcCreateContext");
    if (oal_alcCreateContext == NULL)
        return 0;
    return oal_alcCreateContext(device, attrs);
}

int LIBMATTI_OAL_ALC_MakeContextCurrent(long context)
{
    oal_probe();
    if (!oal_available)
        return 0;
    OAL_DLSYM(alcMakeContextCurrent, "alcMakeContextCurrent");
    if (oal_alcMakeContextCurrent == NULL)
        return 0;
    return oal_alcMakeContextCurrent(context);
}

void LIBMATTI_OAL_ALC_DestroyContext(long context)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alcDestroyContext, "alcDestroyContext");
    if (oal_alcDestroyContext == NULL)
        return;
    oal_alcDestroyContext(context);
}

int LIBMATTI_OAL_ALC_GetError(long device)
{
    oal_probe();
    if (!oal_available)
        return LIBMATTI_OAL_ALC_INVALID_DEVICE; // the "dead" error keeps callers in the fallback path
    OAL_DLSYM(alcGetError, "alcGetError");
    if (oal_alcGetError == NULL)
        return LIBMATTI_OAL_ALC_INVALID_DEVICE;
    return oal_alcGetError(device);
}

const char *LIBMATTI_OAL_ALC_GetString(long device, int param)
{
    oal_probe();
    if (!oal_available)
        return "";
    OAL_DLSYM(alcGetStringPtr, "alcGetString");
    if (oal_alcGetStringPtr == NULL)
        return "";
    return oal_alcGetStringPtr(device, param);
}

int LIBMATTI_OAL_ALC_IsExtensionPresent(long device, const char *extension)
{
    oal_probe();
    if (!oal_available)
        return 0;
    OAL_DLSYM(alcIsExtensionPresent, "alcIsExtensionPresent");
    if (oal_alcIsExtensionPresent == NULL)
        return 0;
    return oal_alcIsExtensionPresent(device, extension);
}

// ---------------------------------------------------------------------------
// AL
// ---------------------------------------------------------------------------

void LIBMATTI_OAL_AL_GenBuffers(int count, unsigned int *buffers)
{
    oal_probe();
    if (!oal_available)
    {
        if (buffers != NULL)
            for (int i = 0; i < count; i++)
                buffers[i] = 0;
        return;
    }
    OAL_DLSYM(alGenBuffers, "alGenBuffers");
    if (oal_alGenBuffers == NULL || buffers == NULL)
    {
        if (buffers != NULL)
            for (int i = 0; i < count; i++)
                buffers[i] = 0;
        return;
    }
    oal_alGenBuffers(count, buffers);
}

void LIBMATTI_OAL_AL_GenSources(int count, unsigned int *sources)
{
    oal_probe();
    if (!oal_available)
    {
        if (sources != NULL)
            for (int i = 0; i < count; i++)
                sources[i] = 0;
        return;
    }
    OAL_DLSYM(alGenSources, "alGenSources");
    if (oal_alGenSources == NULL || sources == NULL)
    {
        if (sources != NULL)
            for (int i = 0; i < count; i++)
                sources[i] = 0;
        return;
    }
    oal_alGenSources(count, sources);
}

void LIBMATTI_OAL_AL_DeleteBuffers(int count, const unsigned int *buffers)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alDeleteBuffers, "alDeleteBuffers");
    if (oal_alDeleteBuffers == NULL || buffers == NULL)
        return;
    oal_alDeleteBuffers(count, buffers);
}

void LIBMATTI_OAL_AL_DeleteSources(int count, const unsigned int *sources)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alDeleteSources, "alDeleteSources");
    if (oal_alDeleteSources == NULL || sources == NULL)
        return;
    oal_alDeleteSources(count, sources);
}

void LIBMATTI_OAL_AL_BufferData(unsigned int buffer, int format, const void *data,
                                 int sizeBytes, int sampleRate)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alBufferData, "alBufferData");
    if (oal_alBufferData == NULL)
        return;
    oal_alBufferData(buffer, format, data, sizeBytes, sampleRate);
}

void LIBMATTI_OAL_AL_SourcePlay(unsigned int source)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourcePlay, "alSourcePlay");
    if (oal_alSourcePlay == NULL || source == 0)
        return;
    oal_alSourcePlay(source);
}

void LIBMATTI_OAL_AL_SourceStop(unsigned int source)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourceStop, "alSourceStop");
    if (oal_alSourceStop == NULL || source == 0)
        return;
    oal_alSourceStop(source);
}

void LIBMATTI_OAL_AL_SourcePause(unsigned int source)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourcePause, "alSourcePause");
    if (oal_alSourcePause == NULL || source == 0)
        return;
    oal_alSourcePause(source);
}

void LIBMATTI_OAL_AL_Sourcei(unsigned int source, int param, int value)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourcei, "alSourcei");
    if (oal_alSourcei == NULL || source == 0)
        return;
    oal_alSourcei(source, param, value);
}

void LIBMATTI_OAL_AL_Sourcef(unsigned int source, int param, float value)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourcef, "alSourcef");
    if (oal_alSourcef == NULL || source == 0)
        return;
    oal_alSourcef(source, param, value);
}

void LIBMATTI_OAL_AL_Source3f(unsigned int source, int param, float x, float y, float z)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSource3f, "alSource3f");
    if (oal_alSource3f == NULL || source == 0)
        return;
    oal_alSource3f(source, param, x, y, z);
}

void LIBMATTI_OAL_AL_Sourcefv(unsigned int source, int param, const float *values)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourcefv, "alSourcefv");
    if (oal_alSourcefv == NULL || source == 0 || values == NULL)
        return;
    oal_alSourcefv(source, param, values);
}

int LIBMATTI_OAL_AL_GetSourcei(unsigned int source, int param)
{
    oal_probe();
    if (!oal_available)
        return 0;
    OAL_DLSYM(alGetSourcei, "alGetSourcei");
    if (oal_alGetSourcei == NULL || source == 0)
        return 0;
    int value = 0;
    oal_alGetSourcei(source, param, &value);
    return value;
}

void LIBMATTI_OAL_AL_SourceQueueBuffers(unsigned int source, int count, const unsigned int *buffers)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourceQueueBuffers, "alSourceQueueBuffers");
    if (oal_alSourceQueueBuffers == NULL || source == 0)
        return;
    oal_alSourceQueueBuffers(source, count, buffers);
}

void LIBMATTI_OAL_AL_SourceUnqueueBuffers(unsigned int source, int count, unsigned int *buffers)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alSourceUnqueueBuffers, "alSourceUnqueueBuffers");
    if (oal_alSourceUnqueueBuffers == NULL || source == 0)
        return;
    oal_alSourceUnqueueBuffers(source, count, buffers);
}

void LIBMATTI_OAL_AL_Listener3f(int param, float x, float y, float z)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alListener3f, "alListener3f");
    if (oal_alListener3f == NULL)
        return;
    oal_alListener3f(param, x, y, z);
}

void LIBMATTI_OAL_AL_Listenerfv(int param, const float *values)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alListenerfv, "alListenerfv");
    if (oal_alListenerfv == NULL || values == NULL)
        return;
    oal_alListenerfv(param, values);
}

void LIBMATTI_OAL_AL_Listenerf(int param, float value)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alListenerf, "alListenerf");
    if (oal_alListenerf == NULL)
        return;
    oal_alListenerf(param, value);
}

void LIBMATTI_OAL_AL_DistanceModel(int model)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alDistanceModel, "alDistanceModel");
    if (oal_alDistanceModel == NULL)
        return;
    oal_alDistanceModel(model);
}

void LIBMATTI_OAL_AL_DopplerFactor(float value)
{
    oal_probe();
    if (!oal_available)
        return;
    OAL_DLSYM(alDopplerFactor, "alDopplerFactor");
    if (oal_alDopplerFactor == NULL)
        return;
    oal_alDopplerFactor(value);
}

int LIBMATTI_OAL_AL_GetError(void)
{
    oal_probe();
    if (!oal_available)
        return LIBMATTI_OAL_AL_NO_ERROR; // the dead binding stays silent;
    OAL_DLSYM(alGetError, "alGetError");
    if (oal_alGetError == NULL)
        return LIBMATTI_OAL_AL_NO_ERROR; // the dead binding stays silent;
    return oal_alGetError();
}

const char *LIBMATTI_OAL_AL_GetString(int param)
{
    oal_probe();
    if (!oal_available)
        return "";;
    OAL_DLSYM(alGetString, "alGetString");
    if (oal_alGetString == NULL)
        return "";;
    return oal_alGetString(param);
}

int LIBMATTI_OAL_AL_IsSource(unsigned int source)
{
    oal_probe();
    if (!oal_available)
        return 0;;
    OAL_DLSYM(alIsSource, "alIsSource");
    if (oal_alIsSource == NULL || source == 0)
        return 0;;
    return oal_alIsSource(source);
}

int LIBMATTI_OAL_AL_IsBuffer(unsigned int buffer)
{
    oal_probe();
    if (!oal_available)
        return 0;;
    OAL_DLSYM(alIsBuffer, "alIsBuffer");
    if (oal_alIsBuffer == NULL || buffer == 0)
        return 0;;
    return oal_alIsBuffer(buffer);
}

// ---------------------------------------------------------------------------
// capability surface
// ---------------------------------------------------------------------------

int LIBMATTI_OAL_HasExtLinearDistance(void)
{
    // Java: ALCapabilities.AL_EXT_LINEAR_DISTANCE - the AL extension the
    // Library.init requires (the channels' linearAttenuation rides it).
    oal_probe();
    if (!oal_available)
        return 0;
    // The AL extension list rides the AL context (alGetString(AL_EXTENSIONS)).
    const char *extensions = LIBMATTI_OAL_AL_GetString(LIBMATTI_OAL_AL_EXTENSIONS);
    if (extensions == NULL)
        return 0;
    // Java: the capability flag is a substring check over the extension list.
    const char *found = strstr(extensions, "AL_EXT_LINEAR_DISTANCE");
    return found != NULL;
}

char **LIBMATTI_OAL_ALC_GetAllDeviceSpecifiers(void)
{
    // Java: ALUtil.getStringList(device, ALC_ALL_DEVICES_SPECIFIER) - the ALC
    // returns one NUL-separated block; the port splits it into the C-string
    // array (NULL when the extension is missing).
    oal_probe();
    if (!oal_available)
        return NULL;
    // Java: ALC_ALL_DEVICES_SPECIFIER (the ALC_ENUMERATE_ALL_EXT extension).
#define OAL_ALC_ALL_DEVICES_SPECIFIER 0x1013
    const char *block = LIBMATTI_OAL_ALC_GetString(0, OAL_ALC_ALL_DEVICES_SPECIFIER);
    if (block == NULL || block[0] == '\0')
        return NULL;
    int count = 0;
    const char *cursor = block;
    while (*cursor != '\0')
    {
        count++;
        cursor += strlen(cursor) + 1;
    }
    char **list = calloc((size_t) count + 1, sizeof(char *));
    if (list == NULL)
        return NULL;
    cursor = block;
    for (int i = 0; i < count; i++)
    {
        list[i] = strdup(cursor);
        cursor += strlen(cursor) + 1;
    }
    return list;
}

void LIBMATTI_OAL_ALC_FreeDeviceSpecifiers(char **specifiers)
{
    if (specifiers == NULL)
        return;
    for (int i = 0; specifiers[i] != NULL; i++)
        free(specifiers[i]);
    free(specifiers);
}

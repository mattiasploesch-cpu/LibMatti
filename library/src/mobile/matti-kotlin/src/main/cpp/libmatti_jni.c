// JNI bridge for the Android build. The renderer selection is the same
// layer:api string the desktop uses; libmatti detects Android/arm64 itself.
//
// The main CMake library glob compiles every C file under library/src, so the
// file is empty in desktop builds (jni.h only exists in the Android NDK).

#if defined(__ANDROID__)

#include <jni.h>
#include <stdlib.h>
#include <string.h>

#include "libmatti/matti/Platform.h"
#include "libmatti/matti/ui/bridge.h"

// Exported by the Rust window crate (library/src/libmatti/rs).
extern int matti_rs_init(int argc, const char *const *argv);

static char **build_argv(JNIEnv *env, jobjectArray args, int *out_count)
{
    int count = args != NULL ? (*env)->GetArrayLength(env, args) : 0;
    char **argv = calloc((size_t) (count + 2), sizeof(char *));
    if (argv == NULL)
        return NULL;
    argv[0] = strdup("matticraft");
    for (int i = 0; i < count; i++)
    {
        jstring string = (jstring) (*env)->GetObjectArrayElement(env, args, i);
        const char *utf = (*env)->GetStringUTFChars(env, string, NULL);
        argv[i + 1] = strdup(utf != NULL ? utf : "");
        if (utf != NULL)
            (*env)->ReleaseStringUTFChars(env, string, utf);
        (*env)->DeleteLocalRef(env, string);
    }
    *out_count = count + 1;
    return argv;
}

JNIEXPORT jstring JNICALL
Java_dev_matticraft_libmatti_LibMatti_defaultSelection(JNIEnv *env, jobject self)
{
    (void) self;
    return (*env)->NewStringUTF(env, LIBMATTI_Platform_DefaultRenderer());
}

JNIEXPORT jstring JNICALL
Java_dev_matticraft_libmatti_LibMatti_platformName(JNIEnv *env, jobject self)
{
    (void) self;
    return (*env)->NewStringUTF(env, LIBMATTI_Platform_Name());
}

JNIEXPORT jint JNICALL
Java_dev_matticraft_libmatti_LibMatti_run(JNIEnv *env, jobject self, jobjectArray args)
{
    (void) self;
    int argc = 0;
    char **argv = build_argv(env, args, &argc);
    if (argv == NULL)
        return 2;

    int status = matti_rs_init(argc, (const char *const *) argv);
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
    return status;
}

JNIEXPORT void JNICALL
Java_dev_matticraft_libmatti_LibMatti_setSurface(JNIEnv *env, jobject self, jobject surface)
{
    (void) env;
    (void) self;
    (void) surface;
    // M2: hand ANativeWindow_fromSurface() to the SDL/Android window backend.
}

JNIEXPORT void JNICALL
Java_dev_matticraft_libmatti_LibMatti_resize(JNIEnv *env, jobject self, jint width, jint height)
{
    (void) env;
    (void) self;
    (void) width;
    (void) height;
    // M2: forward to the renderer's logical framebuffer resize.
}

JNIEXPORT void JNICALL
Java_dev_matticraft_libmatti_LibMatti_pressEscape(JNIEnv *env, jobject self)
{
    (void) env;
    (void) self;
    // M2: inject KEY_ESCAPE into the C menu input state.
}

JNIEXPORT void JNICALL
Java_dev_matticraft_libmatti_LibMatti_shutdown(JNIEnv *env, jobject self)
{
    (void) env;
    (void) self;
    LIBMATTI_UI_RequestClose();
}

#endif // __ANDROID__

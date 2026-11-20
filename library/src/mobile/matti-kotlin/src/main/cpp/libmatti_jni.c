






#if defined(__ANDROID__)

#include <jni.h>
#include <stdlib.h>
#include <string.h>

#include <android/native_window_jni.h>

#include "libmatti/matti/Platform.h"
#include "libmatti/matti/render/android_surface.h"
#include "libmatti/matti/ui/bridge.h"



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
    (void) self;
    ANativeWindow *window = surface != NULL ? ANativeWindow_fromSurface(env, surface) : NULL;
    LIBMATTI_Android_SetSurface(window);
}

JNIEXPORT void JNICALL
Java_dev_matticraft_libmatti_LibMatti_resize(JNIEnv *env, jobject self, jint width, jint height)
{
    (void) env;
    (void) self;
    LIBMATTI_Android_SetSize(width, height);
}

JNIEXPORT void JNICALL
Java_dev_matticraft_libmatti_LibMatti_pressEscape(JNIEnv *env, jobject self)
{
    (void) env;
    (void) self;
    LIBMATTI_UI_RequestClose();
}

JNIEXPORT void JNICALL
Java_dev_matticraft_libmatti_LibMatti_shutdown(JNIEnv *env, jobject self)
{
    (void) env;
    (void) self;
    LIBMATTI_UI_RequestClose();
}

#endif

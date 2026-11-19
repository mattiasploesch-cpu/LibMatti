








#include "libmatti/com/electronwill/nightconfig/core/file/FileWatcher.h"

#include <dirent.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if defined(HAVE_SYS_INOTIFY_H) && HAVE_SYS_INOTIFY_H
#include <sys/inotify.h>
#define MATTI_FILE_WATCHER_INOTIFY 1
#else
#define MATTI_FILE_WATCHER_INOTIFY 0
#endif

// Java: private static final Duration DEFAULT_DEBOUNCE_TIME = Duration.ofMillis(500)
#define DEFAULT_DEBOUNCE_MILLIS 500
// Java: private static final Duration DEFAULT_SERVICE_POLL_TIMEOUT = Duration.ofMillis(200)
#define SERVICE_POLL_TIMEOUT_MILLIS 200

// Java: the ConcurrentMap<FileSystem, FsWatcher> watchers folds into one file table:
//       the watched file, its change handler and its debounce countdown
typedef struct WatchedFile
{
    char *path;
    int watchDescriptor;
    long long lastWriteMillis;
    long long lastSize;
    int hasSnapshot;
// Java: the handler runnable
    void (*changeHandler)(void *self, void *userdata);
    void *handlerSelf;
    void *handlerUserdata;
// Java: the debounce countdown; 0 = no pending event
    long long debounceRemainingMillis;
    struct WatchedFile *next;
} WatchedFile;

struct LIBMATTI_NC_FileWatcher
{
// Java: private volatile boolean running = true
    int running;
    long long debounceMillis;
    int inotifyFd;
    int useInotify;
    pthread_t thread;
    int threadStarted;
    WatchedFile *files;
    pthread_mutex_t lock;
};

// Java: the monotonic clock drives the debounce countdown
static long long now_millis(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long long) now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static long long file_write_millis(const struct stat *status)
{
#if defined(__APPLE__)
    return (long long) status->st_mtimespec.tv_sec * 1000 + status->st_mtimespec.tv_nsec / 1000000;
#elif defined(_WIN32)
    return (long long) status->st_mtime * 1000;
#else
    return (long long) status->st_mtim.tv_sec * 1000 + status->st_mtim.tv_nsec / 1000000;
#endif
}

// Java: WatchService events mark the file dirty; the polling fallback compares the
// file metadata on every service poll instead.
static void remember_file_state(WatchedFile *file)
{
    struct stat status;
    file->hasSnapshot = stat(file->path, &status) == 0;
    if (!file->hasSnapshot) return;
    file->lastWriteMillis = file_write_millis(&status);
    file->lastSize = (long long) status.st_size;
}

static WatchedFile *find_file(LIBMATTI_NC_FileWatcher *watcher, const char *path)
{
    for (WatchedFile *file = watcher->files; file != NULL; file = file->next)
        if (strcmp(file->path, path) == 0) return file;
    return NULL;
}

// Java: public void addWatch(Path file, Runnable changeHandler) -> addOrPutWatch(ADD)
LIBMATTI_NC_FileWatcher_WatchingException *LIBMATTI_NC_FileWatcher_AddWatch(
    LIBMATTI_NC_FileWatcher *watcher, const char *file,
    void (*changeHandler)(void *self, void *userdata), void *changeHandlerSelf, void *changeHandlerUserdata)
{
    if (watcher == NULL || !watcher->running) return NULL;

// Java: the file's parent directory must exist (WatchingException otherwise)
    char *directory = strdup(file);
    char *slash = strrchr(directory, '/');
    if (slash != NULL)
        *slash = '\0';
    else
        directory[0] = '.';
    DIR *dir = opendir(directory[0] != '\0' ? directory : "/");
    free(directory);
    if (dir == NULL)
    {
        LIBMATTI_NC_FileWatcher_WatchingException *exception = calloc(1, sizeof(*exception));
        exception->message = malloc(strlen(file) + 64);
        sprintf(exception->message, "The parent directory of %s does not exist", file);
        return exception;
    }
    closedir(dir);

    pthread_mutex_lock(&watcher->lock);

// Java: Watches a file, if not already watched by this FileWatcher
    WatchedFile *existing = find_file(watcher, file);
    if (existing != NULL)
    {
        existing->changeHandler = changeHandler;
        existing->handlerSelf = changeHandlerSelf;
        existing->handlerUserdata = changeHandlerUserdata;
        pthread_mutex_unlock(&watcher->lock);
        return NULL;
    }

// Java: WatchService.register(path) - the inotify watch on the file itself
    int watchDescriptor = -1;
#if MATTI_FILE_WATCHER_INOTIFY
    if (watcher->useInotify)
    {
        watchDescriptor = inotify_add_watch(watcher->inotifyFd, file,
                                            IN_MODIFY | IN_CREATE | IN_DELETE | IN_ATTRIB);
        if (watchDescriptor < 0)
        {
            pthread_mutex_unlock(&watcher->lock);
            LIBMATTI_NC_FileWatcher_WatchingException *exception = calloc(1, sizeof(*exception));
            exception->message = malloc(strlen(file) + 64);
            sprintf(exception->message, "Failed to watch %s: %s", file, strerror(errno));
            return exception;
        }
    }
#endif

    WatchedFile *watched = calloc(1, sizeof(*watched));
    watched->path = strdup(file);
    watched->watchDescriptor = watchDescriptor;
    remember_file_state(watched);
    watched->changeHandler = changeHandler;
    watched->handlerSelf = changeHandlerSelf;
    watched->handlerUserdata = changeHandlerUserdata;
    watched->debounceRemainingMillis = 0;
    watched->next = watcher->files;
    watcher->files = watched;

    pthread_mutex_unlock(&watcher->lock);
    return NULL;
}

// Java: public void removeWatch(Path file)
void LIBMATTI_NC_FileWatcher_RemoveWatch(LIBMATTI_NC_FileWatcher *watcher, const char *file)
{
    if (watcher == NULL) return;

    pthread_mutex_lock(&watcher->lock);
    WatchedFile **link = &watcher->files;
    while (*link != NULL)
    {
        WatchedFile *file_entry = *link;
        if (strcmp(file_entry->path, file) == 0)
        {
            *link = file_entry->next;
#if MATTI_FILE_WATCHER_INOTIFY
            if (watcher->useInotify)
                inotify_rm_watch(watcher->inotifyFd, file_entry->watchDescriptor);
#endif
            free(file_entry->path);
            free(file_entry);
            continue;
        }
        link = &file_entry->next;
    }
    pthread_mutex_unlock(&watcher->lock);
}

// Java: the FsWatcher.run loop - poll for events, run handlers whose debounce expired
static void *watcher_thread(void *userdata)
{
    LIBMATTI_NC_FileWatcher *watcher = userdata;
#if MATTI_FILE_WATCHER_INOTIFY
    char buffer[sizeof(struct inotify_event) + 4096];
#endif

    long long lastPoll = now_millis();
    while (watcher->running)
    {
// Java: WatchService.poll(servicePollTimeout) - the control channel timeout
        int ready = 0;
#if !MATTI_FILE_WATCHER_INOTIFY
        (void) ready;
#endif
#if MATTI_FILE_WATCHER_INOTIFY
        if (watcher->useInotify)
        {
            struct pollfd fds = {.fd = watcher->inotifyFd, .events = POLLIN};
            ready = poll(&fds, 1, SERVICE_POLL_TIMEOUT_MILLIS);
        }
        else
#endif
        {
            struct timespec timeout = {0, SERVICE_POLL_TIMEOUT_MILLIS * 1000000L};
            nanosleep(&timeout, NULL);
        }
        long long now = now_millis();
        long long elapsed = now - lastPoll;
        lastPoll = now;

        pthread_mutex_lock(&watcher->lock);
#if MATTI_FILE_WATCHER_INOTIFY
        if (watcher->useInotify && ready > 0)
        {
            ssize_t length = read(watcher->inotifyFd, buffer, sizeof(buffer));
            for (ssize_t offset = 0; watcher->running && length > 0 && offset < length;)
            {
                struct inotify_event *event = (struct inotify_event *) (buffer + offset);
                offset += (ssize_t) (sizeof(struct inotify_event) + event->len);

// Java: any modification resets the countdown of the watched file
                for (WatchedFile *file = watcher->files; file != NULL; file = file->next)
                    if (file->watchDescriptor == event->wd)
                        file->debounceRemainingMillis = watcher->debounceMillis;
            }
        }
#endif

        if (!watcher->useInotify)
        {
            for (WatchedFile *file = watcher->files; file != NULL; file = file->next)
            {
                struct stat status;
                int exists = stat(file->path, &status) == 0;
                long long modified = exists ? file_write_millis(&status) : 0;
                long long size = exists ? (long long) status.st_size : 0;
                if (exists != file->hasSnapshot ||
                    (exists && (modified != file->lastWriteMillis || size != file->lastSize)))
                {
                    file->hasSnapshot = exists;
                    file->lastWriteMillis = modified;
                    file->lastSize = size;
// Java: any modification resets the countdown of the watched file
                    file->debounceRemainingMillis = watcher->debounceMillis;
                }
            }
        }

// Java: the debounce thread fires the handler once the calm period expired
        for (WatchedFile *file = watcher->files; file != NULL; file = file->next)
        {
            if (file->debounceRemainingMillis <= 0) continue;
            file->debounceRemainingMillis -= elapsed;
            if (file->debounceRemainingMillis <= 0)
            {
                file->debounceRemainingMillis = 0;
// Java: the handler runs on the watcher thread
                if (file->changeHandler != NULL)
                    file->changeHandler(file->handlerSelf, file->handlerUserdata);
            }
        }
        pthread_mutex_unlock(&watcher->lock);
    }
    return NULL;
}

// Java: public FileWatcher() - creates and starts the watcher
LIBMATTI_NC_FileWatcher *LIBMATTI_NC_FileWatcher_New(void)
{
    return LIBMATTI_NC_FileWatcher_NewWithDebounce(DEFAULT_DEBOUNCE_MILLIS);
}

LIBMATTI_NC_FileWatcher *LIBMATTI_NC_FileWatcher_NewWithDebounce(long long debounceMillis)
{
    LIBMATTI_NC_FileWatcher *watcher = calloc(1, sizeof(*watcher));
    watcher->running = 1;
    watcher->debounceMillis = debounceMillis;
    watcher->inotifyFd = -1;
#if MATTI_FILE_WATCHER_INOTIFY
    watcher->inotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    watcher->useInotify = watcher->inotifyFd >= 0;
#endif
    pthread_mutex_init(&watcher->lock, NULL);

// Java: the watcher thread starts with the instance (the thread group "watchers-<id>")
    watcher->threadStarted = pthread_create(&watcher->thread, NULL, watcher_thread, watcher) == 0;
    return watcher;
}

// Java: public static synchronized FileWatcher defaultInstance() - created on demand
LIBMATTI_NC_FileWatcher *LIBMATTI_NC_FileWatcher_DefaultInstance(void)
{
    static LIBMATTI_NC_FileWatcher *defaultInstance = NULL;
    if (defaultInstance == NULL)
        defaultInstance = LIBMATTI_NC_FileWatcher_New();
    return defaultInstance;
}

// Java: public void stop()
void LIBMATTI_NC_FileWatcher_Stop(LIBMATTI_NC_FileWatcher *watcher)
{
    if (watcher == NULL || !watcher->running) return;
    watcher->running = 0;
    if (watcher->threadStarted)
        pthread_join(watcher->thread, NULL);
}

void LIBMATTI_NC_FileWatcher_Free(LIBMATTI_NC_FileWatcher *watcher)
{
    if (watcher == NULL) return;
    LIBMATTI_NC_FileWatcher_Stop(watcher);
    while (watcher->files != NULL)
    {
        WatchedFile *file = watcher->files;
        watcher->files = file->next;
        free(file->path);
        free(file);
    }
    if (watcher->inotifyFd >= 0) close(watcher->inotifyFd);
    pthread_mutex_destroy(&watcher->lock);
    free(watcher);
}

void LIBMATTI_NC_FileWatcher_WatchingException_Free(LIBMATTI_NC_FileWatcher_WatchingException *exception)
{
    if (exception == NULL) return;
    free(exception->message);
    free(exception);
}

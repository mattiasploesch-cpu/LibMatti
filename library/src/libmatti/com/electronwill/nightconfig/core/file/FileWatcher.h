// Port of com.electronwill.nightconfig.core.file.FileWatcher.
// Java watches files asynchronously on a WatchService thread with a debounce time;
// every modification resets the countdown and the change handler runs after the calm
// period. The port implements the same lifecycle over inotify (Linux): one watcher
// thread, one watch per file, one debounce timer per file, handlers invoked on the
// watcher thread exactly like Java's watchers-<id> thread group.

#ifndef MATTICRAFT_NC_FILEWATCHER_H
#define MATTICRAFT_NC_FILEWATCHER_H

#include <stddef.h>

// Java: FileWatcher.WatchingException - reported when a watch cannot be set up
typedef struct
{
    char *message;
} LIBMATTI_NC_FileWatcher_WatchingException;

// Java: public final class FileWatcher
typedef struct LIBMATTI_NC_FileWatcher LIBMATTI_NC_FileWatcher;

// Java: public static synchronized FileWatcher defaultInstance()
LIBMATTI_NC_FileWatcher *LIBMATTI_NC_FileWatcher_DefaultInstance(void);

// Java: public FileWatcher() - the default debounce time (500ms in night-config)
LIBMATTI_NC_FileWatcher *LIBMATTI_NC_FileWatcher_New(void);
LIBMATTI_NC_FileWatcher *LIBMATTI_NC_FileWatcher_NewWithDebounce(long long debounceMillis);

// Java: public void addWatch(Path file, Runnable changeHandler)
// WatchingException when the file's parent directory does not exist; the C port
// reports that through the returned WatchingException (caller frees), NULL = ok.
LIBMATTI_NC_FileWatcher_WatchingException *LIBMATTI_NC_FileWatcher_AddWatch(
    LIBMATTI_NC_FileWatcher *watcher, const char *file,
    void (*changeHandler)(void *self, void *userdata), void *changeHandlerSelf, void *changeHandlerUserdata);

// Java: public void removeWatch(Path file)
void LIBMATTI_NC_FileWatcher_RemoveWatch(LIBMATTI_NC_FileWatcher *watcher, const char *file);

// Java: private void stopWatchers() / public void stop() - stops the watcher thread
void LIBMATTI_NC_FileWatcher_Stop(LIBMATTI_NC_FileWatcher *watcher);

void LIBMATTI_NC_FileWatcher_Free(LIBMATTI_NC_FileWatcher *watcher);

// Java: public static class WatchingException
void LIBMATTI_NC_FileWatcher_WatchingException_Free(LIBMATTI_NC_FileWatcher_WatchingException *exception);

#endif //MATTICRAFT_NC_FILEWATCHER_H

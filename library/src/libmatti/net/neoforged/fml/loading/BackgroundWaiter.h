




#ifndef MATTICRAFT_FML_LOADING_BACKGROUNDWAITER_H
#define MATTICRAFT_FML_LOADING_BACKGROUNDWAITER_H

// Java: Runnable
typedef void (*LIBMATTI_FML_BackgroundWaiter_Task)(void *userdata);

// Java: public static void runAndTick(Runnable r, Runnable tick)
void LIBMATTI_FML_BackgroundWaiter_RunAndTick(LIBMATTI_FML_BackgroundWaiter_Task r, void *rUserdata,
                                             LIBMATTI_FML_BackgroundWaiter_Task tick, void *tickUserdata);

#endif

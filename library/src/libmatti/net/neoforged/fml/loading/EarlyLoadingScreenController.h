// Port of net.neoforged.fml.loading.EarlyLoadingScreenController.

#ifndef MATTICRAFT_FML_LOADING_EARLYLOADINGSCREENCONTROLLER_H
#define MATTICRAFT_FML_LOADING_EARLYLOADINGSCREENCONTROLLER_H

// Java: public interface EarlyLoadingScreenController
typedef struct LIBMATTI_FML_EarlyLoadingScreenController LIBMATTI_FML_EarlyLoadingScreenController;

struct LIBMATTI_FML_EarlyLoadingScreenController
{
    void *self;

    // Java: long takeOverGlfwWindow()
    long (*takeOverGlfwWindow)(void *self);
    // Java: void periodicTick()
    void (*periodicTick)(void *self);
    // Java: void updateProgress(String label)
    void (*updateProgress)(void *self, const char *label);
    // Java: void completeProgress()
    void (*completeProgress)(void *self);
};

// Java: static EarlyLoadingScreenController current()
LIBMATTI_FML_EarlyLoadingScreenController *LIBMATTI_FML_EarlyLoadingScreenController_Current(void);

// Java: public long takeOverGlfwWindow()
long LIBMATTI_FML_EarlyLoadingScreenController_TakeOverGlfwWindow(const LIBMATTI_FML_EarlyLoadingScreenController *controller);
// Java: public void periodicTick()
void LIBMATTI_FML_EarlyLoadingScreenController_PeriodicTick(const LIBMATTI_FML_EarlyLoadingScreenController *controller);
// Java: public void updateProgress(String label)
void LIBMATTI_FML_EarlyLoadingScreenController_UpdateProgress(const LIBMATTI_FML_EarlyLoadingScreenController *controller,
                                                              const char *label);
// Java: public void completeProgress()
void LIBMATTI_FML_EarlyLoadingScreenController_CompleteProgress(const LIBMATTI_FML_EarlyLoadingScreenController *controller);

#endif //MATTICRAFT_FML_LOADING_EARLYLOADINGSCREENCONTROLLER_H

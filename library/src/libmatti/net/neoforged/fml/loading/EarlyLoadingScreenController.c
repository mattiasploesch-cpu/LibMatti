#include "libmatti/net/neoforged/fml/loading/EarlyLoadingScreenController.h"

#include "libmatti/net/neoforged/fml/loading/ImmediateWindowHandler.h"

// Java: static EarlyLoadingScreenController current() { return ImmediateWindowHandler.provider; }
LIBMATTI_FML_EarlyLoadingScreenController *LIBMATTI_FML_EarlyLoadingScreenController_Current(void)
{
    return LIBMATTI_FML_ImmediateWindowHandler_GetProvider();
}

// Java: public long takeOverGlfwWindow()
long LIBMATTI_FML_EarlyLoadingScreenController_TakeOverGlfwWindow(
    const LIBMATTI_FML_EarlyLoadingScreenController *controller)
{
    return controller->takeOverGlfwWindow(controller->self);
}

// Java: public void periodicTick()
void LIBMATTI_FML_EarlyLoadingScreenController_PeriodicTick(
    const LIBMATTI_FML_EarlyLoadingScreenController *controller)
{
    controller->periodicTick(controller->self);
}

// Java: public void updateProgress(String label)
void LIBMATTI_FML_EarlyLoadingScreenController_UpdateProgress(
    const LIBMATTI_FML_EarlyLoadingScreenController *controller, const char *label)
{
    controller->updateProgress(controller->self, label);
}

// Java: public void completeProgress()
void LIBMATTI_FML_EarlyLoadingScreenController_CompleteProgress(
    const LIBMATTI_FML_EarlyLoadingScreenController *controller)
{
    controller->completeProgress(controller->self);
}

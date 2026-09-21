// Port of net.neoforged.fml.loading.mixin.FMLClassTracker.
// "Tracks invalid (unloadable) classes so we can throw an exception inside the TCL and class
// load events so we can report when classes were loaded before we could transform them"

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSTRACKER_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSTRACKER_H

#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

// Java: class FMLClassTracker implements IClassTracker
LIBMATTI_SP_IClassTracker *LIBMATTI_FML_FMLClassTracker_Instance(void);
// Java: boolean isInvalidClass(String className) - the package-private getter
int LIBMATTI_FML_FMLClassTracker_IsInvalidClass(const char *className);
// Java: void addLoadedClass(String className) - the package-private setter
void LIBMATTI_FML_FMLClassTracker_AddLoadedClass(const char *className);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLCLASSTRACKER_H

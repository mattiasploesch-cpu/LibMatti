//
// Port of cpw.mods.modlauncher.Environment.
//

#ifndef MATTICRAFT_MODLAUNCHER_ENVIRONMENT_H
#define MATTICRAFT_MODLAUNCHER_ENVIRONMENT_H

#include "libmatti/cpw/modlauncher/api/IEnvironment.h"

struct LIBMATTI_ML_Launcher;

// Java: public final class Environment implements IEnvironment
typedef struct LIBMATTI_ML_Environment
{
    LIBMATTI_MLA_TypesafeMap *environment;
    struct LIBMATTI_ML_Launcher *launcher;
} LIBMATTI_ML_Environment;

// Java: Environment(Launcher launcher)
LIBMATTI_ML_Environment *LIBMATTI_ML_Environment_New(struct LIBMATTI_ML_Launcher *launcher);

// Java: <T> T computePropertyIfAbsent(TypesafeMap.Key<T> key,
//                                    Function<? super TypesafeMap.Key<T>, ? extends T> valueFunction)
void *LIBMATTI_ML_Environment_ComputePropertyIfAbsent(LIBMATTI_ML_Environment *environment, LIBMATTI_MLA_Key *key,
                                                      void *(*valueFunction)(LIBMATTI_MLA_Key *key, void *userdata),
                                                      void *userdata);

#endif //MATTICRAFT_MODLAUNCHER_ENVIRONMENT_H

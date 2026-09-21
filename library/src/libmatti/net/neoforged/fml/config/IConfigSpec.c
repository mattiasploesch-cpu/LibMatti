// Port of net.neoforged.fml.config.IConfigSpec.

#include "libmatti/net/neoforged/fml/config/IConfigSpec.h"

LIBMATTI_FML_IConfigSpec LIBMATTI_FML_IConfigSpec_Of(void *self,
                                                     int (*isEmpty)(void *self),
                                                     void (*validateSpec)(void *self, void *config),
                                                     int (*isCorrect)(void *self, const LIBMATTI_NC_Config *config),
                                                     void (*correct)(void *self, LIBMATTI_NC_Config *config),
                                                     void (*acceptConfig)(void *self,
                                                                          LIBMATTI_FML_ILoadedConfig *config))
{
    LIBMATTI_FML_IConfigSpec spec;
    spec.self = self;
    spec.isEmpty = isEmpty;
    spec.validateSpec = validateSpec;
    spec.isCorrect = isCorrect;
    spec.correct = correct;
    spec.acceptConfig = acceptConfig;
    return spec;
}

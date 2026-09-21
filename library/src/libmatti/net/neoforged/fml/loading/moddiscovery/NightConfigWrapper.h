// Port of net.neoforged.fml.loading.moddiscovery.NightConfigWrapper.
// com.electronwill.nightconfig.core.UnmodifiableConfig is the ported
// LIBMATTI_NC_Config (com/electronwill/nightconfig/core/Config.h).

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_NIGHTCONFIGWRAPPER_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_NIGHTCONFIGWRAPPER_H

#include "libmatti/com/electronwill/nightconfig/core/Config.h"
#include "libmatti/java/util/HashMap.h"
#include "libmatti/net/neoforged/neoforgespi/language/IConfigurable.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_NightConfigWrapper LIBMATTI_FML_NightConfigWrapper;

// Java: public NightConfigWrapper(UnmodifiableConfig config)
LIBMATTI_FML_NightConfigWrapper *LIBMATTI_FML_NightConfigWrapper_New(LIBMATTI_NC_Config *config);
// Java: public NightConfigWrapper setFile(IModFileInfo file)
LIBMATTI_FML_NightConfigWrapper *LIBMATTI_FML_NightConfigWrapper_SetFile(
    LIBMATTI_FML_NightConfigWrapper *wrapper, LIBMATTI_NEOFORGESPI_IModFileInfo *file);
// Java: the wrapper used as an IConfigurable
LIBMATTI_NEOFORGESPI_IConfigurable *LIBMATTI_FML_NightConfigWrapper_AsConfigurable(
    LIBMATTI_FML_NightConfigWrapper *wrapper);
// Java: the method reference configWrapper::setFile
void LIBMATTI_FML_NightConfigWrapper_SetFileConsumer(LIBMATTI_NEOFORGESPI_IModFileInfo *file, void *userdata);
void LIBMATTI_FML_NightConfigWrapper_Free(LIBMATTI_FML_NightConfigWrapper *wrapper);

// Java: public <T> Optional<T> getConfigElement(String... key)
// Java: ((UnmodifiableConfig) value).valueMap() - the Map<String, Object> view of a config table;
// keys and string values are copied, nested configs stay LIBMATTI_NC_Config handles
LIBMATTI_JU_HashMap *LIBMATTI_FML_NightConfigWrapper_ValueMap(LIBMATTI_NC_Config *config);

int LIBMATTI_FML_NightConfigWrapper_GetConfigElement(void *self, const char *const *key, size_t keyCount,
                                                     LIBMATTI_NEOFORGESPI_IConfigurable_Value *out);
// Java: public List<? extends IConfigurable> getConfigList(String... key)
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_FML_NightConfigWrapper_GetConfigList(void *self, const char *const *key,
                                                                                    size_t keyCount, size_t *count);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_NIGHTCONFIGWRAPPER_H

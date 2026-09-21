#include "libmatti/net/neoforged/neoforgespi/language/IConfigurable.h"

// Java: <T> Optional<T> getConfigElement(String... key)
int LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigElement(
    const LIBMATTI_NEOFORGESPI_IConfigurable *configurable, const char *const *key, size_t keyCount,
    LIBMATTI_NEOFORGESPI_IConfigurable_Value *out)
{
    return configurable->getConfigElement(configurable->self, key, keyCount, out);
}

// Java: List<? extends IConfigurable> getConfigList(String... key)
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigList(
    const LIBMATTI_NEOFORGESPI_IConfigurable *configurable, const char *const *key, size_t keyCount, size_t *count)
{
    return configurable->getConfigList(configurable->self, key, keyCount, count);
}

// Java: getConfigElement(key) for a single-element key
int LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(const LIBMATTI_NEOFORGESPI_IConfigurable *configurable,
                                                  const char *key,
                                                  LIBMATTI_NEOFORGESPI_IConfigurable_Value *out)
{
    return LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigElement(configurable, &key, 1, out);
}

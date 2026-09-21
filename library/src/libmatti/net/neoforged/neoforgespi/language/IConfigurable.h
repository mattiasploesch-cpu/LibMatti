// Port of net.neoforged.neoforgespi.language.IConfigurable.
// Java infers T at the call site; the C port returns a tagged value instead.

#ifndef MATTICRAFT_NEOFORGESPI_LANGUAGE_ICONFIGURABLE_H
#define MATTICRAFT_NEOFORGESPI_LANGUAGE_ICONFIGURABLE_H

#include <stddef.h>

typedef enum
{
    LIBMATTI_NEOFORGESPI_IConfigurable_STRING,
    LIBMATTI_NEOFORGESPI_IConfigurable_BOOL,
    LIBMATTI_NEOFORGESPI_IConfigurable_INT,
    // Java: Map<String, Object>
    LIBMATTI_NEOFORGESPI_IConfigurable_MAP,
    LIBMATTI_NEOFORGESPI_IConfigurable_STRING_LIST,
    // Java: com.electronwill.nightconfig.core.UnmodifiableConfig
    LIBMATTI_NEOFORGESPI_IConfigurable_CONFIG
} LIBMATTI_NEOFORGESPI_IConfigurable_ValueType;

typedef struct
{
    LIBMATTI_NEOFORGESPI_IConfigurable_ValueType type;
    const char *string; // STRING
    int boolean; // BOOL
    int integer; // INT
    void *map; // MAP / CONFIG (opaque)
    char **list; // STRING_LIST
    size_t listCount; // STRING_LIST
} LIBMATTI_NEOFORGESPI_IConfigurable_Value;

// Java: public interface IConfigurable
typedef struct LIBMATTI_NEOFORGESPI_IConfigurable LIBMATTI_NEOFORGESPI_IConfigurable;

struct LIBMATTI_NEOFORGESPI_IConfigurable
{
    void *self;

    // Java: <T> Optional<T> getConfigElement(String... key)
    int (*getConfigElement)(void *self, const char *const *key, size_t keyCount,
                            LIBMATTI_NEOFORGESPI_IConfigurable_Value *out);
    // Java: List<? extends IConfigurable> getConfigList(String... key)
    LIBMATTI_NEOFORGESPI_IConfigurable **(*getConfigList)(void *self, const char *const *key, size_t keyCount,
                                                          size_t *count);
};

// Java: <T> Optional<T> getConfigElement(String... key)
int LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigElement(
    const LIBMATTI_NEOFORGESPI_IConfigurable *configurable, const char *const *key, size_t keyCount,
    LIBMATTI_NEOFORGESPI_IConfigurable_Value *out);
// Java: List<? extends IConfigurable> getConfigList(String... key)
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_NEOFORGESPI_IConfigurable_GetConfigList(
    const LIBMATTI_NEOFORGESPI_IConfigurable *configurable, const char *const *key, size_t keyCount, size_t *count);

// Java: getConfigElement(key) for a single-element key
int LIBMATTI_NEOFORGESPI_IConfigurable_GetElement(const LIBMATTI_NEOFORGESPI_IConfigurable *configurable,
                                                  const char *key,
                                                  LIBMATTI_NEOFORGESPI_IConfigurable_Value *out);

#endif //MATTICRAFT_NEOFORGESPI_LANGUAGE_ICONFIGURABLE_H

// Port of net.neoforged.fml.loading.mixin.FMLClassTracker.
// Java: the two ConcurrentHashMap-backed sets; the port keeps two string sets.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLClassTracker.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    char **names;
    size_t count;
} NameSet;

static NameSet invalidClasses = {NULL, 0};
static NameSet loadedClasses = {NULL, 0};

static int set_contains(const NameSet *set, const char *name)
{
    for (size_t i = 0; i < set->count; i++)
        if (strcmp(set->names[i], name) == 0) return 1;
    return 0;
}

static void set_add(NameSet *set, const char *name)
{
    if (set_contains(set, name)) return;

    set->names = realloc(set->names, sizeof(char *) * (set->count + 1));
    set->names[set->count++] = strdup(name);
}

// Java: @Override public void registerInvalidClass(String className)
static void register_invalid_class(void *self, const char *className)
{
    (void) self;
    set_add(&invalidClasses, className);
}

// Java: @Override public boolean isClassLoaded(String className)
static int is_class_loaded(void *self, const char *className)
{
    (void) self;
    return set_contains(&loadedClasses, className);
}

// Java: @Override public String getClassRestrictions(String className) { return ""; }
static const char *get_class_restrictions(void *self, const char *className)
{
    (void) self;
    (void) className;
    return "";
}

// Java: class FMLClassTracker implements IClassTracker
LIBMATTI_SP_IClassTracker *LIBMATTI_FML_FMLClassTracker_Instance(void)
{
    static LIBMATTI_SP_IClassTracker tracker;
    static int initialised = 0;

    if (!initialised)
    {
        tracker.self = NULL;
        tracker.registerInvalidClass = register_invalid_class;
        tracker.isClassLoaded = is_class_loaded;
        tracker.getClassRestrictions = get_class_restrictions;
        initialised = 1;
    }
    return &tracker;
}

// Java: boolean isInvalidClass(String className) { return this.invalidClasses.contains(className); }
int LIBMATTI_FML_FMLClassTracker_IsInvalidClass(const char *className)
{
    return set_contains(&invalidClasses, className);
}

// Java: void addLoadedClass(String className) { this.loadedClasses.add(className); }
void LIBMATTI_FML_FMLClassTracker_AddLoadedClass(const char *className)
{
    set_add(&loadedClasses, className);
}

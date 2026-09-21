// Port of net.neoforged.fml.loading.mixin.FMLAuditTrail.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLAuditTrail.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final String APPLY_MIXIN_ACTIVITY = "APP"; etc.
#define ACTIVITY_APPLY "APP"
#define ACTIVITY_POST_PROCESS "DEC"
#define ACTIVITY_GENERATE "GEN"

typedef struct
{
    // Java: private String currentClass
    char *currentClass;
    // Java: private BiConsumer<String, String[]> consumer
    void (*consumer)(const char *activity, const char **context, size_t contextCount, void *userdata);
    void *userdata;
} FMLAuditTrail;

static FMLAuditTrail auditTrail = {NULL, NULL, NULL};

// Java: private void writeActivity(String className, String activity, String... context) {
//           if (this.consumer != null && className.equals(this.currentClass)) this.consumer.accept(activity, context); }
static void write_activity(const char *className, const char *activity, const char **context, size_t contextCount)
{
    if (auditTrail.consumer != NULL && auditTrail.currentClass != NULL &&
        strcmp(className, auditTrail.currentClass) == 0)
    {
        auditTrail.consumer(activity, context, contextCount, auditTrail.userdata);
    }
}

static void on_apply(void *self, const char *className, const char *mixinName)
{
    (void) self;
    const char *context[] = {mixinName};
    write_activity(className, ACTIVITY_APPLY, context, 1);
}

static void on_post_process(void *self, const char *className)
{
    (void) self;
    write_activity(className, ACTIVITY_POST_PROCESS, NULL, 0);
}

static void on_generate(void *self, const char *className, const char *generatorName)
{
    (void) self;
    write_activity(className, ACTIVITY_GENERATE, NULL, 0);
}

// Java: public void setConsumer(String className, BiConsumer<String, String[]> consumer)
static void set_consumer(void *self, const char *className,
                         void (*consumer)(const char *activity, const char **context, size_t contextCount,
                                          void *userdata), void *userdata)
{
    (void) self;

    free(auditTrail.currentClass);
    auditTrail.currentClass = className != NULL ? strdup(className) : NULL;
    auditTrail.consumer = consumer;
    auditTrail.userdata = userdata;
}

// Java: class FMLAuditTrail implements IMixinAuditTrail
LIBMATTI_SP_IMixinAuditTrail *LIBMATTI_FML_FMLAuditTrail_Instance(void)
{
    static LIBMATTI_SP_IMixinAuditTrail trail;
    static int initialised = 0;

    if (!initialised)
    {
        trail.self = NULL;
        trail.onApply = on_apply;
        trail.onPostProcess = on_post_process;
        trail.onGenerate = on_generate;
        trail.setConsumer = set_consumer;
        initialised = 1;
    }
    return &trail;
}

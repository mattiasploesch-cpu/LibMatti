// Port of net.neoforged.neoforgespi.transformation.ClassProcessorIds.

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

#include <stdlib.h>

// Java: the static initializers of the public static final ProcessorName fields
static LIBMATTI_NEOFORGESPI_ProcessorName *new_id(const char *namespace, const char *path)
{
    return LIBMATTI_NEOFORGESPI_ProcessorName_New(namespace, path);
}

const LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES(void)
{
    static LIBMATTI_NEOFORGESPI_ProcessorName *name = NULL;
    if (name == NULL) name = new_id("neoforge", "computing_frames");
    return name;
}

const LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessorIds_SIMPLE_PROCESSORS_GROUP(void)
{
    static LIBMATTI_NEOFORGESPI_ProcessorName *name = NULL;
    if (name == NULL) name = new_id("neoforge", "simple_processors_default");
    return name;
}

const LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessorIds_RUNTIME_ENUM_EXTENDER(void)
{
    static LIBMATTI_NEOFORGESPI_ProcessorName *name = NULL;
    if (name == NULL) name = new_id("neoforge", "runtime_enum_extender");
    return name;
}

const LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessorIds_ACCESS_TRANSFORMERS(void)
{
    static LIBMATTI_NEOFORGESPI_ProcessorName *name = NULL;
    if (name == NULL) name = new_id("neoforge", "access_transformer");
    return name;
}

const LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN(void)
{
    static LIBMATTI_NEOFORGESPI_ProcessorName *name = NULL;
    if (name == NULL) name = new_id("neoforge", "mixin");
    return name;
}

const LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN_FRAME_CONTEXT(void)
{
    static LIBMATTI_NEOFORGESPI_ProcessorName *name = NULL;
    if (name == NULL) name = new_id("neoforge", "mixin_frame_context");
    return name;
}

const LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ClassProcessorIds_DIST_CLEANER(void)
{
    static LIBMATTI_NEOFORGESPI_ProcessorName *name = NULL;
    if (name == NULL) name = new_id("neoforge", "neoforge_dev_dist_cleaner");
    return name;
}

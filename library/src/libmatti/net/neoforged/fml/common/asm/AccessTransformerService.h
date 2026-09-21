// Port of net.neoforged.fml.common.asm.AccessTransformerService.

#ifndef MATTICRAFT_FML_COMMON_ASM_ACCESSTRANSFORMERSERVICE_H
#define MATTICRAFT_FML_COMMON_ASM_ACCESSTRANSFORMERSERVICE_H

#include "libmatti/net/neoforged/accesstransformer/api/AccessTransformerEngine.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

// Java: public class AccessTransformerService implements ClassProcessor
typedef struct LIBMATTI_FML_AccessTransformerService
{
    // Java: implements ClassProcessor
    LIBMATTI_NEOFORGESPI_ClassProcessor processor;

    // Java: private final AccessTransformerEngine engine
    LIBMATTI_AT_AccessTransformerEngine *engine;
} LIBMATTI_FML_AccessTransformerService;

// Java: public AccessTransformerService(AccessTransformerEngine engine)
LIBMATTI_FML_AccessTransformerService *LIBMATTI_FML_AccessTransformerService_New(
    LIBMATTI_AT_AccessTransformerEngine *engine);
void LIBMATTI_FML_AccessTransformerService_Free(LIBMATTI_FML_AccessTransformerService *service);

#endif //MATTICRAFT_FML_COMMON_ASM_ACCESSTRANSFORMERSERVICE_H

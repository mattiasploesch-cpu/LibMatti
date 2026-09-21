// Port of net.neoforged.fml.common.asm.enumextension.EnumPrototype.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPROTOTYPE_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPROTOTYPE_H

#include "libmatti/net/neoforged/fml/common/asm/enumextension/EnumParameters.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarResource.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"

#include <stddef.h>

// Java: record EnumPrototype(String owningMod, String enumName, String fieldName, String ctorDesc,
//         String fullCtorDesc, EnumParameters ctorParams) implements Comparable<EnumPrototype>
typedef struct LIBMATTI_FML_EnumPrototype
{
    char *owningMod;
    char *enumName;
    char *fieldName;
    char *ctorDesc;
    char *fullCtorDesc;
    LIBMATTI_FML_EnumParameters *ctorParams;
} LIBMATTI_FML_EnumPrototype;

// Java: private static final String ENUM_CTOR_BASE_DESC = "Ljava/lang/String;I"
#define LIBMATTI_FML_ENUMPROTOTYPE_ENUM_CTOR_BASE_DESC "Ljava/lang/String;I"

// Java: static List<EnumPrototype> load(IModInfo mod, JarResource resource); caller frees
LIBMATTI_FML_EnumPrototype *LIBMATTI_FML_EnumPrototype_Load(const LIBMATTI_NEOFORGESPI_IModInfo *mod,
                                                           const LIBMATTI_FML_JarResource *resource,
                                                           size_t *count);
// Java: public int compareTo(EnumPrototype other) -> -1, 0, 1
int LIBMATTI_FML_EnumPrototype_Compare(const LIBMATTI_FML_EnumPrototype *prototype,
                                       const LIBMATTI_FML_EnumPrototype *other);

void LIBMATTI_FML_EnumPrototype_Free(LIBMATTI_FML_EnumPrototype *prototype);

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPROTOTYPE_H

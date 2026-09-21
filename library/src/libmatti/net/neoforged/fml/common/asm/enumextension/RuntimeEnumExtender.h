// Port of net.neoforged.fml.common.asm.enumextension.RuntimeEnumExtender.
// Java: a ClassProcessor that transforms enums implementing IExtensibleEnum to add additional
// entries loaded from files provided by mods.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_RUNTIMEENUMEXTENDER_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_RUNTIMEENUMEXTENDER_H

#include "libmatti/net/neoforged/fml/common/asm/enumextension/EnumPrototype.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarResource.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"
#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"
#include "libmatti/org/objectweb/asm/tree/MethodNode.h"

#include <stddef.h>

// Java: public class RuntimeEnumExtender implements ClassProcessor
typedef struct LIBMATTI_FML_RuntimeEnumExtender
{
    // Java: implements ClassProcessor
    LIBMATTI_NEOFORGESPI_ClassProcessor processor;
} LIBMATTI_FML_RuntimeEnumExtender;

// Java: public RuntimeEnumExtender()
LIBMATTI_FML_RuntimeEnumExtender *LIBMATTI_FML_RuntimeEnumExtender_New(void);
void LIBMATTI_FML_RuntimeEnumExtender_Free(LIBMATTI_FML_RuntimeEnumExtender *extender);

// Java: public static MethodInsnNode findFirstStaticMethodCall(MethodNode method, String owner, String name, String descriptor)
LIBMATTI_ASMT_MethodInsnNode *LIBMATTI_FML_RuntimeEnumExtender_FindFirstStaticMethodCall(
    const LIBMATTI_ASMT_MethodNode *method, const char *owner, const char *name, const char *descriptor);

// Java: public static AbstractInsnNode findFirstInstructionBefore(MethodNode method, int opCode, int startIndex)
LIBMATTI_ASMT_AbstractInsnNode *LIBMATTI_FML_RuntimeEnumExtender_FindFirstInstructionBefore(
    const LIBMATTI_ASMT_MethodNode *method, int opCode, long long startIndex);

// Java: public static void loadEnumPrototypes(Map<IModInfo, JarResource> paths)
void LIBMATTI_FML_RuntimeEnumExtender_LoadEnumPrototypes(const LIBMATTI_NEOFORGESPI_IModInfo **mods,
                                                        const LIBMATTI_FML_JarResource **resources, size_t count);

// Java: public static String validateNameParameter(String fieldName, String owningMod)
const char *LIBMATTI_FML_RuntimeEnumExtender_ValidateNameParameter(const char *fieldName, const char *owningMod);

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_RUNTIMEENUMEXTENDER_H

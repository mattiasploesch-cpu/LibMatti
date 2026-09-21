// Port of net.neoforged.accesstransformer.api.AccessTransformerEngine
// (+ AccessTransformerEngineImpl and AccessTransformerList).

#ifndef MATTICRAFT_AT_ACCESSTRANSFORMERENGINE_H
#define MATTICRAFT_AT_ACCESSTRANSFORMERENGINE_H

#include "libmatti/net/neoforged/accesstransformer/parser/AtParser.h"
#include "libmatti/net/neoforged/accesstransformer/parser/Target.h"
#include "libmatti/net/neoforged/accesstransformer/parser/Transformation.h"
#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

#include <stddef.h>

// Java: public interface AccessTransformerEngine + the impl's masterList.
// The engine owns one flat list of (target, transformation) entries; lookups
// scan for the internal class name.
typedef struct LIBMATTI_AT_AccessTransformerEngine LIBMATTI_AT_AccessTransformerEngine;

// Java: static AccessTransformerEngine newEngine()
LIBMATTI_AT_AccessTransformerEngine *LIBMATTI_AT_AccessTransformerEngine_New(void);
void LIBMATTI_AT_AccessTransformerEngine_Free(LIBMATTI_AT_AccessTransformerEngine *engine);

// Java: void loadAT(Reader reader, String originName) - text = the file content.
// Returns 0 on a parse error; *errorLine gets the 1-based line.
int LIBMATTI_AT_AccessTransformerEngine_LoadAT(LIBMATTI_AT_AccessTransformerEngine *engine,
                                               const char *text, const char *originName, int *errorLine);
// Java: void loadATFromPath(Path path)
int LIBMATTI_AT_AccessTransformerEngine_LoadATFromPath(LIBMATTI_AT_AccessTransformerEngine *engine,
                                                       const char *path, int *errorLine);
// Java: void loadATFromResource(String resourceName)
int LIBMATTI_AT_AccessTransformerEngine_LoadATFromResource(LIBMATTI_AT_AccessTransformerEngine *engine,
                                                           const char *resourceName, int *errorLine);

// Java: Set<Type> getTargets() - the distinct targeted class internal names
char **LIBMATTI_AT_AccessTransformerEngine_GetTargets(const LIBMATTI_AT_AccessTransformerEngine *engine,
                                                      size_t *count);
// Java: boolean containsClassTarget(Type type)
int LIBMATTI_AT_AccessTransformerEngine_ContainsClassTarget(const LIBMATTI_AT_AccessTransformerEngine *engine,
                                                            const char *internalName);
// Java: Set<String> getSourcesForTarget(String className, TargetType type, String targetName)
char **LIBMATTI_AT_AccessTransformerEngine_GetSourcesForTarget(const LIBMATTI_AT_AccessTransformerEngine *engine,
                                                               const char *internalName, LIBMATTI_AT_TargetType type,
                                                               const char *targetName, size_t *count);

// Java: boolean transform(ClassNode classNode, Type name) - applies the ATs;
// returns 1 when the class was modified
int LIBMATTI_AT_AccessTransformerEngine_Transform(LIBMATTI_AT_AccessTransformerEngine *engine,
                                                  LIBMATTI_ASMT_ClassNode *classNode, const char *internalName);

#endif //MATTICRAFT_AT_ACCESSTRANSFORMERENGINE_H

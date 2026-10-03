// Port of net.minecraft.util.datafix.DataFixers (P7.3) - the fix registry the
// game builds once (the singleton behind DataFixers.getDataFixer()).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_DATAFIXERS_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_DATAFIXERS_H

#include "libmatti/com/mojang/datafixers/DataFixer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static DataFixer getDataFixer() - the built fixer, null before
// the first call. The port builds it lazily on first use, like the Java class
// initializer does for DATA_FIXER.
const LIBMATTI_MC_DataFixer *LIBMATTI_MC_DataFixers_GetDataFixer(void);

// Java: DataFixers.optimize(...) has no C counterpart (the rule cache is not
// ported); this releases the singleton so the process can shut down clean.
void LIBMATTI_MC_DataFixers_Free(void);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_DATAFIXERS_H
// Port of the com.mojang.datafixers.DataFixUtils helpers the port needs
// (P7.3).

#ifndef MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIXUTILS_H
#define MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIXUTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: DataFixUtils.ceillog2(int) - the smallest bits with (1 << bits) >= value
int LIBMATTI_MC_DataFixUtils_CeilLog2(int value);

// Java: DataFixUtils.makeKey(int version) - the schema version key
int64_t LIBMATTI_MC_DataFixUtils_MakeKey(int version);

// Java: DataFixUtils.getVersion(long key) - the DataVersion out of the key
int LIBMATTI_MC_DataFixUtils_GetVersion(int64_t key);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIXUTILS_H
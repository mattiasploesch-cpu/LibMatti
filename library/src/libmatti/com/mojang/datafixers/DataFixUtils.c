// Port of the com.mojang.datafixers.DataFixUtils helpers the port needs
// (P7.3).

#include "libmatti/com/mojang/datafixers/DataFixUtils.h"

int LIBMATTI_MC_DataFixUtils_CeilLog2(int value)
{
    // Java: int i = 0; while ((1 << i) < p_value) { i++; } return i;
    int bits = 0;
    while ((1 << bits) < value)
        bits++;
    return bits;
}

int64_t LIBMATTI_MC_DataFixUtils_MakeKey(int version)
{
    // Java: (long)version << 8 | 0 - the version key carries the version in
    // the high bits and the schema sub-version in the low eight
    return ((int64_t) version) << 8;
}

int LIBMATTI_MC_DataFixUtils_GetVersion(int64_t key)
{
    // Java: (int)(p_key >> 8)
    return (int) (key >> 8);
}
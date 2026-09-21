// Port of net.minecraft.util.InclusiveRange.

#include "libmatti/net/minecraft/util/InclusiveRange.h"

#include <stdlib.h>

LIBMATTI_MC_InclusiveRange LIBMATTI_MC_InclusiveRange_Of(int minInclusive, int maxInclusive)
{
    // Java: Preconditions.checkState(min <= max) - the port reports and aborts
    if (minInclusive > maxInclusive)
    {
        abort();
    }
    LIBMATTI_MC_InclusiveRange range = {minInclusive, maxInclusive};
    return range;
}

LIBMATTI_MC_InclusiveRange LIBMATTI_MC_InclusiveRange_OfSingle(int value)
{
    return LIBMATTI_MC_InclusiveRange_Of(value, value);
}

int LIBMATTI_MC_InclusiveRange_IsValueInRange(const LIBMATTI_MC_InclusiveRange *range, int value)
{
    return value >= range->minInclusive && value <= range->maxInclusive;
}

LIBMATTI_MC_InclusiveRange LIBMATTI_MC_InclusiveRange_Cloned(const LIBMATTI_MC_InclusiveRange *range)
{
    return *range;
}

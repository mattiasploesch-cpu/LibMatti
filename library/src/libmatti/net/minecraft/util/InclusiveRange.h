// Port of net.minecraft.util.InclusiveRange (the T = int and T = PackFormat uses).

#ifndef MATTICRAFT_MC_UTIL_INCLUSIVERANGE_H
#define MATTICRAFT_MC_UTIL_INCLUSIVERANGE_H

// Java: public record InclusiveRange<T extends Comparable<T>>(T minInclusive, T maxInclusive)
// The port specialises on int and on (major, minor) pairs like PackFormat.
typedef struct
{
    int minInclusive;
    int maxInclusive;
} LIBMATTI_MC_InclusiveRange;

// Java: public InclusiveRange(T minInclusive, T maxInclusive) - asserts min <= max
LIBMATTI_MC_InclusiveRange LIBMATTI_MC_InclusiveRange_Of(int minInclusive, int maxInclusive);
// Java: public InclusiveRange(T value) - a single-value range
LIBMATTI_MC_InclusiveRange LIBMATTI_MC_InclusiveRange_OfSingle(int value);

// Java: public boolean isValueInRange(T value)
int LIBMATTI_MC_InclusiveRange_IsValueInRange(const LIBMATTI_MC_InclusiveRange *range, int value);
// Java: public InclusiveRange<T> map(Function<T, U>) for the int identity
LIBMATTI_MC_InclusiveRange LIBMATTI_MC_InclusiveRange_Cloned(const LIBMATTI_MC_InclusiveRange *range);

#endif //MATTICRAFT_MC_UTIL_INCLUSIVERANGE_H

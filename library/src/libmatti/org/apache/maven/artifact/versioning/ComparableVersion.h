// Port of org.apache.maven.artifact.versioning.ComparableVersion.
// Java's Item/IntItem/LongItem/BigIntegerItem/StringItem/CombinationItem/ListItem become one tagged
// LIBMATTI_MAVEN_Item. Java's BigInteger is kept as its decimal digits and compared numerically by
// length then lexically.
// TODO: ComparableVersion.main / ListItem.toListString - the debug CLI has no counterpart.

#ifndef MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_COMPARABLEVERSION_H
#define MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_COMPARABLEVERSION_H

#include <stddef.h>

typedef struct LIBMATTI_MAVEN_ComparableVersion LIBMATTI_MAVEN_ComparableVersion;

// Java: private interface Item { INT_ITEM, LONG_ITEM, BIGINTEGER_ITEM, STRING_ITEM, LIST_ITEM, COMBINATION_ITEM }
typedef enum
{
    LIBMATTI_MAVEN_Item_BIG_INTEGER = 0,
    LIBMATTI_MAVEN_Item_STRING = 1,
    LIBMATTI_MAVEN_Item_LIST = 2,
    LIBMATTI_MAVEN_Item_INT = 3,
    LIBMATTI_MAVEN_Item_LONG = 4,
    LIBMATTI_MAVEN_Item_COMBINATION = 5
} LIBMATTI_MAVEN_ItemType;

typedef struct LIBMATTI_MAVEN_Item LIBMATTI_MAVEN_Item;

struct LIBMATTI_MAVEN_Item
{
    LIBMATTI_MAVEN_ItemType type;

    // IntItem
    int intValue;
    // LongItem
    long longValue;
    // BigIntegerItem
    char *digits;
    // StringItem
    char *stringValue;
    // CombinationItem
    LIBMATTI_MAVEN_Item *stringPart;
    LIBMATTI_MAVEN_Item *digitPart;
    // ListItem
    LIBMATTI_MAVEN_Item **items;
    size_t count;
    size_t capacity;
};

// Java: public ComparableVersion(String version)
LIBMATTI_MAVEN_ComparableVersion *LIBMATTI_MAVEN_ComparableVersion_New(const char *version);
void LIBMATTI_MAVEN_ComparableVersion_Free(LIBMATTI_MAVEN_ComparableVersion *version);

// Java: public int compareTo(ComparableVersion o)
int LIBMATTI_MAVEN_ComparableVersion_Compare(const LIBMATTI_MAVEN_ComparableVersion *a,
                                            const LIBMATTI_MAVEN_ComparableVersion *b);
// Java: public int hashCode()
int LIBMATTI_MAVEN_ComparableVersion_HashCode(const LIBMATTI_MAVEN_ComparableVersion *version);
// Java: public int orderingHashCode()
int LIBMATTI_MAVEN_ComparableVersion_OrderingHashCode(const LIBMATTI_MAVEN_ComparableVersion *version);
// Java: public String toString()
const char *LIBMATTI_MAVEN_ComparableVersion_ToString(const LIBMATTI_MAVEN_ComparableVersion *version);
// Java: public String getCanonical()
char *LIBMATTI_MAVEN_ComparableVersion_GetCanonical(LIBMATTI_MAVEN_ComparableVersion *version);

#endif //MATTICRAFT_MAVEN_ARTIFACT_VERSIONING_COMPARABLEVERSION_H

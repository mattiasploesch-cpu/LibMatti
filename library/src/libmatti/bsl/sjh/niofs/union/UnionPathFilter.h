//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.niofs.union.UnionPathFilter.

#ifndef MATTICRAFT_UNIONPATHFILTER_H
#define MATTICRAFT_UNIONPATHFILTER_H

// Java: @FunctionalInterface UnionPathFilter
// C has no functional interfaces; the filter is a function pointer plus user data.
typedef struct
{
    // Java: boolean test(String entry, Path basePath)
    // entry: path relative to the base path; basePath: one of the root paths
    // Returns 1 to include the entry, 0 to exclude it.
    int (*test)(const char *entry, const char *basePath, void *userdata);
    void *userdata;
} LIBMATTI_UNION_UnionPathFilter;

#endif //MATTICRAFT_UNIONPATHFILTER_H
// Java: static VertexSorting byDistance(float x, float y, float z) - a sorting
// lambda bound to a center point. The port wraps the shared core with the
// center captured into a static per-call table (single-threaded render path,
// one binding at a time like Java's lambda captures).

#include "libmatti/com/mojang/blaze3d/vertex/VertexSorting.h"

#include "libmatti/com/mojang/blaze3d/vertex/CompactVectorArray.h"

#include <stdlib.h>

static float binding[3];

static int *by_distance_impl(const LIBMATTI_B3D_CompactVectorArray *centroids, size_t *outCount)
{
    return LIBMATTI_B3D_VertexSorting_SortByDistance(centroids, binding[0], binding[1], binding[2], 0, outCount);
}

LIBMATTI_B3D_VertexSorting_Sort LIBMATTI_B3D_VertexSorting_ByDistance(float x, float y, float z)
{
    binding[0] = x;
    binding[1] = y;
    binding[2] = z;
    return by_distance_impl;
}

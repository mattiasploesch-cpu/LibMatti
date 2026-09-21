// Port of com.mojang.blaze3d.vertex.VertexSorting - the transparent quad sort.
// Java is a functional interface; the C port uses a function pointer plus the
// two built-in singletons DISTANCE_TO_ORIGIN and ORTHOGRAPHIC_Z.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_VERTEXSORTING_H
#define MATTICRAFT_BLAZE3D_VERTEX_VERTEXSORTING_H

#include <stddef.h>

struct LIBMATTI_B3D_CompactVectorArray;

#ifdef __cplusplus
extern "C"
{
#endif

// Java: @FunctionalInterface public interface VertexSorting -
// int[] sort(CompactVectorArray) returns the quad indices, far to near.
typedef int *(*LIBMATTI_B3D_VertexSorting_Sort)(const struct LIBMATTI_B3D_CompactVectorArray *centroids,
                                                size_t *outCount);

// Java: VertexSorting.DISTANCE_TO_ORIGIN - byDistance(0, 0, 0).
LIBMATTI_B3D_VertexSorting_Sort LIBMATTI_B3D_VertexSorting_DistanceToOrigin(void);
// Java: VertexSorting.ORTHOGRAPHIC_Z - byDistance(v -> -v.z()).
LIBMATTI_B3D_VertexSorting_Sort LIBMATTI_B3D_VertexSorting_OrthographicZ(void);

// Java: static VertexSorting byDistance(float, float, float) - a caller-owned
// comparator built around a center point.
LIBMATTI_B3D_VertexSorting_Sort LIBMATTI_B3D_VertexSorting_ByDistance(float x, float y, float z);

// The sort implementations allocate the index array; free() the result.
// Shared core (Java: byDistance(DistanceFunction)) - sorts far to near.
int *LIBMATTI_B3D_VertexSorting_SortByDistance(const struct LIBMATTI_B3D_CompactVectorArray *centroids,
                                               float x, float y, float z, int invertZ, size_t *outCount);

#ifdef __cplusplus
}
#endif

#endif

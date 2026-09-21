#include "libmatti/com/mojang/blaze3d/vertex/VertexSorting.h"

#include "libmatti/com/mojang/blaze3d/vertex/CompactVectorArray.h"

#include <stdlib.h>

// Java: byDistance(DistanceFunction) - sorts the indices by the squared
// distance in descending order (far to near) with a stable merge sort.
int *LIBMATTI_B3D_VertexSorting_SortByDistance(const LIBMATTI_B3D_CompactVectorArray *centroids,
                                               float x, float y, float z, int invertZ, size_t *outCount)
{
    int count = LIBMATTI_B3D_CompactVectorArray_Size(centroids);
    float *distances = malloc((size_t) count * sizeof(float));
    int *indices = malloc((size_t) count * sizeof(int));

    for (int i = 0; i < count; i++)
    {
        indices[i] = i;
        float dx = LIBMATTI_B3D_CompactVectorArray_GetX(centroids, i) - x;
        float dy = LIBMATTI_B3D_CompactVectorArray_GetY(centroids, i) - y;
        float dz = LIBMATTI_B3D_CompactVectorArray_GetZ(centroids, i) - z;
        // Java: distanceSquared; ORTHOGRAPHIC_Z negates the z term through the
        // distance function (byDistance(p -> -p.z())).
        if (invertZ)
            dz = -dz;
        distances[i] = dx * dx + dy * dy + dz * dz;
    }

    // Java: IntArrays.mergeSort(aint, (a, b) -> Floats.compare(afloat[b], afloat[a]))
    // - descending by distance, stable.
    for (int width = 1; width < count; width *= 2)
    {
        int *temp = malloc((size_t) count * sizeof(int));
        for (int left = 0; left < count; left += 2 * width)
        {
            int mid = left + width < count ? left + width : count;
            int right = left + 2 * width < count ? left + 2 * width : count;
            int i = left, j = mid, k = left;
            while (i < mid && j < right)
            {
                if (distances[indices[j]] > distances[indices[i]])
                    temp[k++] = indices[j++];
                else
                    temp[k++] = indices[i++];
            }
            while (i < mid) temp[k++] = indices[i++];
            while (j < right) temp[k++] = indices[j++];
        }
        for (int i = 0; i < count; i++)
            indices[i] = temp[i];
        free(temp);
    }

    free(distances);
    if (outCount != NULL)
        *outCount = (size_t) count;
    return indices;
}

int *LIBMATTI_B3D_VertexSorting_DistanceToOrigin_Impl(const LIBMATTI_B3D_CompactVectorArray *centroids,
                                                      size_t *outCount)
{
    // Java: byDistance(0.0F, 0.0F, 0.0F).
    return LIBMATTI_B3D_VertexSorting_SortByDistance(centroids, 0.0f, 0.0f, 0.0f, 0, outCount);
}

int *LIBMATTI_B3D_VertexSorting_OrthographicZ_Impl(const LIBMATTI_B3D_CompactVectorArray *centroids,
                                                   size_t *outCount)
{
    // Java: byDistance(p_277433_ -> -p_277433_.z()).
    return LIBMATTI_B3D_VertexSorting_SortByDistance(centroids, 0.0f, 0.0f, 0.0f, 1, outCount);
}

LIBMATTI_B3D_VertexSorting_Sort LIBMATTI_B3D_VertexSorting_DistanceToOrigin(void)
{
    return LIBMATTI_B3D_VertexSorting_DistanceToOrigin_Impl;
}

LIBMATTI_B3D_VertexSorting_Sort LIBMATTI_B3D_VertexSorting_OrthographicZ(void)
{
    return LIBMATTI_B3D_VertexSorting_OrthographicZ_Impl;
}

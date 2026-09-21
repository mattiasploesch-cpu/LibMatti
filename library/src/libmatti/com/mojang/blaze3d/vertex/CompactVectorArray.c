#include "libmatti/com/mojang/blaze3d/vertex/CompactVectorArray.h"

#include <stdlib.h>

LIBMATTI_B3D_CompactVectorArray *LIBMATTI_B3D_CompactVectorArray_New(int size)
{
    LIBMATTI_B3D_CompactVectorArray *array = calloc(1, sizeof(LIBMATTI_B3D_CompactVectorArray));
    // Java: this.contents = new float[3 * p_428520_];
    array->contents = calloc((size_t) size * 3, sizeof(float));
    array->size = size;
    return array;
}

void LIBMATTI_B3D_CompactVectorArray_Free(LIBMATTI_B3D_CompactVectorArray *array)
{
    if (array == NULL)
        return;
    free(array->contents);
    free(array);
}

int LIBMATTI_B3D_CompactVectorArray_Size(const LIBMATTI_B3D_CompactVectorArray *array)
{
    // Java: return this.contents.length / 3;
    return array->size;
}

void LIBMATTI_B3D_CompactVectorArray_Set(LIBMATTI_B3D_CompactVectorArray *array, int index, float x, float y, float z)
{
    array->contents[3 * index + 0] = x;
    array->contents[3 * index + 1] = y;
    array->contents[3 * index + 2] = z;
}

float LIBMATTI_B3D_CompactVectorArray_GetX(const LIBMATTI_B3D_CompactVectorArray *array, int index)
{
    return array->contents[3 * index + 0];
}

float LIBMATTI_B3D_CompactVectorArray_GetY(const LIBMATTI_B3D_CompactVectorArray *array, int index)
{
    return array->contents[3 * index + 1];
}

float LIBMATTI_B3D_CompactVectorArray_GetZ(const LIBMATTI_B3D_CompactVectorArray *array, int index)
{
    // Java has a bug here (returns contents[3*i+1]); the port returns the z slot.
    return array->contents[3 * index + 2];
}

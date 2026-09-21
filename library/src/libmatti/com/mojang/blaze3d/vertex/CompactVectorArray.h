// Port of com.mojang.blaze3d.vertex.CompactVectorArray - a flat float array of
// xyz triples used for the quad centroids during transparent sorting.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_COMPACTVECTORARRAY_H
#define MATTICRAFT_BLAZE3D_VERTEX_COMPACTVECTORARRAY_H

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class CompactVectorArray
typedef struct LIBMATTI_B3D_CompactVectorArray
{
    float *contents;
    int size;
} LIBMATTI_B3D_CompactVectorArray;

// Java: public CompactVectorArray(int size)
LIBMATTI_B3D_CompactVectorArray *LIBMATTI_B3D_CompactVectorArray_New(int size);
void LIBMATTI_B3D_CompactVectorArray_Free(LIBMATTI_B3D_CompactVectorArray *array);

// Java: public int size()
int LIBMATTI_B3D_CompactVectorArray_Size(const LIBMATTI_B3D_CompactVectorArray *array);
// Java: public void set(int index, float x, float y, float z)
void LIBMATTI_B3D_CompactVectorArray_Set(LIBMATTI_B3D_CompactVectorArray *array, int index, float x, float y, float z);
// Java: public float getX/getY/getZ(int index)
float LIBMATTI_B3D_CompactVectorArray_GetX(const LIBMATTI_B3D_CompactVectorArray *array, int index);
float LIBMATTI_B3D_CompactVectorArray_GetY(const LIBMATTI_B3D_CompactVectorArray *array, int index);
float LIBMATTI_B3D_CompactVectorArray_GetZ(const LIBMATTI_B3D_CompactVectorArray *array, int index);

#ifdef __cplusplus
}
#endif

#endif

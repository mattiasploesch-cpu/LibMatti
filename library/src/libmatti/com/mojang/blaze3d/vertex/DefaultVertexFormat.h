// Port of com.mojang.blaze3d.vertex.DefaultVertexFormat - the 15 vanilla
// vertex formats built at class-init. Java holds static final constants; the
// C port hands out lazily created singletons.

#ifndef MATTICRAFT_BLAZE3D_VERTEX_DEFAULTVERTEXFORMAT_H
#define MATTICRAFT_BLAZE3D_VERTEX_DEFAULTVERTEXFORMAT_H

#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: DefaultVertexFormat.EMPTY / BLOCK / NEW_ENTITY / PARTICLE.
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_EMPTY(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_BLOCK(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_NEW_ENTITY(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_PARTICLE(void);

// Java: POSITION, POSITION_COLOR, POSITION_COLOR_NORMAL, POSITION_COLOR_LIGHTMAP.
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR_NORMAL(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR_LIGHTMAP(void);

// Java: POSITION_TEX, POSITION_TEX_COLOR, POSITION_COLOR_TEX_LIGHTMAP,
//       POSITION_TEX_LIGHTMAP_COLOR, POSITION_TEX_COLOR_NORMAL.
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_TEX(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_TEX_COLOR(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR_TEX_LIGHTMAP(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_TEX_LIGHTMAP_COLOR(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_TEX_COLOR_NORMAL(void);

// Java: POSITION_COLOR_LINE_WIDTH, POSITION_COLOR_NORMAL_LINE_WIDTH.
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR_LINE_WIDTH(void);
const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR_NORMAL_LINE_WIDTH(void);

#ifdef __cplusplus
}
#endif

#endif

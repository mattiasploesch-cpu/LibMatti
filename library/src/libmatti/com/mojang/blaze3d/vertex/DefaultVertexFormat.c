#include "libmatti/com/mojang/blaze3d/vertex/DefaultVertexFormat.h"

#include "libmatti/com/mojang/blaze3d/vertex/VertexFormatElement.h"

// Java builds every format through VertexFormat.builder(); the port mirrors
// each builder chain 1:1 and caches the singleton.
#define DEFINE_FORMAT(name)                                                          \
    static LIBMATTI_B3D_VertexFormat *name##_instance = NULL;                        \
    const LIBMATTI_B3D_VertexFormat *LIBMATTI_B3D_DefaultVertexFormat_##name(void)   \
    {                                                                                \
        if (name##_instance == NULL)                                                 \
            name##_instance = name##_build();                                        \
        return name##_instance;                                                      \
    }

static LIBMATTI_B3D_VertexFormat *EMPTY_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(LIBMATTI_B3D_VertexFormat_Builder_New());
}
DEFINE_FORMAT(EMPTY)

static LIBMATTI_B3D_VertexFormat *BLOCK_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Padding(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_Add(
                        LIBMATTI_B3D_VertexFormat_Builder_Add(
                            LIBMATTI_B3D_VertexFormat_Builder_Add(
                                LIBMATTI_B3D_VertexFormat_Builder_New(),
                                "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                            "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
                        "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0),
                    "UV2", &LIBMATTI_B3D_VFE_ELEMENT_UV2),
                "Normal", &LIBMATTI_B3D_VFE_ELEMENT_NORMAL),
            1));
}
DEFINE_FORMAT(BLOCK)

static LIBMATTI_B3D_VertexFormat *NEW_ENTITY_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Padding(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_Add(
                        LIBMATTI_B3D_VertexFormat_Builder_Add(
                            LIBMATTI_B3D_VertexFormat_Builder_Add(
                                LIBMATTI_B3D_VertexFormat_Builder_Add(
                                    LIBMATTI_B3D_VertexFormat_Builder_New(),
                                    "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                                "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
                            "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0),
                        "UV1", &LIBMATTI_B3D_VFE_ELEMENT_UV1),
                    "UV2", &LIBMATTI_B3D_VFE_ELEMENT_UV2),
                "Normal", &LIBMATTI_B3D_VFE_ELEMENT_NORMAL),
            1));
}
DEFINE_FORMAT(NEW_ENTITY)

static LIBMATTI_B3D_VertexFormat *PARTICLE_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_New(),
                    "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0),
            "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR));
}
DEFINE_FORMAT(PARTICLE)

static LIBMATTI_B3D_VertexFormat *POSITION_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(LIBMATTI_B3D_VertexFormat_Builder_New(),
                                              "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION));
}
DEFINE_FORMAT(POSITION)

static LIBMATTI_B3D_VertexFormat *POSITION_COLOR_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_New(),
                "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
            "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR));
}
DEFINE_FORMAT(POSITION_COLOR)

static LIBMATTI_B3D_VertexFormat *POSITION_COLOR_NORMAL_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Padding(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_Add(
                        LIBMATTI_B3D_VertexFormat_Builder_New(),
                        "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                    "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
                "Normal", &LIBMATTI_B3D_VFE_ELEMENT_NORMAL),
            1));
}
DEFINE_FORMAT(POSITION_COLOR_NORMAL)

static LIBMATTI_B3D_VertexFormat *POSITION_COLOR_LIGHTMAP_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_New(),
                    "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
            "UV2", &LIBMATTI_B3D_VFE_ELEMENT_UV2));
}
DEFINE_FORMAT(POSITION_COLOR_LIGHTMAP)

static LIBMATTI_B3D_VertexFormat *POSITION_TEX_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_New(),
                "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
            "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0));
}
DEFINE_FORMAT(POSITION_TEX)

static LIBMATTI_B3D_VertexFormat *POSITION_TEX_COLOR_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_New(),
                    "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0),
            "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR));
}
DEFINE_FORMAT(POSITION_TEX_COLOR)

static LIBMATTI_B3D_VertexFormat *POSITION_COLOR_TEX_LIGHTMAP_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_Add(
                        LIBMATTI_B3D_VertexFormat_Builder_New(),
                        "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                    "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
                "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0),
            "UV2", &LIBMATTI_B3D_VFE_ELEMENT_UV2));
}
DEFINE_FORMAT(POSITION_COLOR_TEX_LIGHTMAP)

static LIBMATTI_B3D_VertexFormat *POSITION_TEX_LIGHTMAP_COLOR_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_Add(
                        LIBMATTI_B3D_VertexFormat_Builder_New(),
                        "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                    "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0),
                "UV2", &LIBMATTI_B3D_VFE_ELEMENT_UV2),
            "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR));
}
DEFINE_FORMAT(POSITION_TEX_LIGHTMAP_COLOR)

static LIBMATTI_B3D_VertexFormat *POSITION_TEX_COLOR_NORMAL_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Padding(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_Add(
                        LIBMATTI_B3D_VertexFormat_Builder_Add(
                            LIBMATTI_B3D_VertexFormat_Builder_New(),
                            "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                        "UV0", &LIBMATTI_B3D_VFE_ELEMENT_UV0),
                    "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
                "Normal", &LIBMATTI_B3D_VFE_ELEMENT_NORMAL),
            1));
}
DEFINE_FORMAT(POSITION_TEX_COLOR_NORMAL)

static LIBMATTI_B3D_VertexFormat *POSITION_COLOR_LINE_WIDTH_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_New(),
                    "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
            "LineWidth", &LIBMATTI_B3D_VFE_ELEMENT_LINE_WIDTH));
}
DEFINE_FORMAT(POSITION_COLOR_LINE_WIDTH)

static LIBMATTI_B3D_VertexFormat *POSITION_COLOR_NORMAL_LINE_WIDTH_build(void)
{
    return LIBMATTI_B3D_VertexFormat_Builder_Build(
        LIBMATTI_B3D_VertexFormat_Builder_Add(
            LIBMATTI_B3D_VertexFormat_Builder_Add(
                LIBMATTI_B3D_VertexFormat_Builder_Add(
                    LIBMATTI_B3D_VertexFormat_Builder_Add(
                        LIBMATTI_B3D_VertexFormat_Builder_New(),
                        "Position", &LIBMATTI_B3D_VFE_ELEMENT_POSITION),
                    "Color", &LIBMATTI_B3D_VFE_ELEMENT_COLOR),
                "Normal", &LIBMATTI_B3D_VFE_ELEMENT_NORMAL),
            "LineWidth", &LIBMATTI_B3D_VFE_ELEMENT_LINE_WIDTH));
}
DEFINE_FORMAT(POSITION_COLOR_NORMAL_LINE_WIDTH)

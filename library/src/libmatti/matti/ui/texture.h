








#ifndef MATTICRAFT_MATTI_UI_TEXTURE_H
#define MATTICRAFT_MATTI_UI_TEXTURE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct LIBMATTI_UI_Texture
{
    int width;
    int height;


    uint32_t *pixels;
} LIBMATTI_UI_Texture;





LIBMATTI_UI_Texture *LIBMATTI_UI_Texture_Load(const char *name);

int LIBMATTI_UI_Texture_Width(const LIBMATTI_UI_Texture *texture);
int LIBMATTI_UI_Texture_Height(const LIBMATTI_UI_Texture *texture);
const uint32_t *LIBMATTI_UI_Texture_Pixels(const LIBMATTI_UI_Texture *texture);




const char *LIBMATTI_UI_Texture_AssetRoot(void);
void LIBMATTI_UI_Texture_SetAssetRoot(const char *root);

#ifdef __cplusplus
}
#endif

#endif

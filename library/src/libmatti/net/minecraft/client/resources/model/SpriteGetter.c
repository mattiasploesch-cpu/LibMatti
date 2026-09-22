// Port of net.minecraft.client.resources.model.SpriteGetter (implementation).

#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"

#include "libmatti/net/minecraft/resources/Identifier.h"

#include <stdlib.h>
#include <string.h>

int LIBMATTI_MC_SpriteGetter_SpriteRect(const LIBMATTI_MC_TextureAtlas *atlas, const char *textureId,
                                        float uvRect[4])
{
    if (atlas == NULL || textureId == NULL)
        return 0;

    // Java: the model texture ids ("block/stone") become atlas Identifiers;
    // a missing namespace defaults to "minecraft" like Identifier.parse.
    LIBMATTI_MC_Identifier *id = strchr(textureId, ':') != NULL
                                     ? LIBMATTI_MC_Identifier_Parse(textureId)
                                     : LIBMATTI_MC_Identifier_FromNamespaceAndPath("minecraft", textureId);
    if (id == NULL)
        return 0;

    // Java: getSprite(Identifier) falls back to the missing sprite.
    LIBMATTI_MC_TextureAtlasSprite *sprite = LIBMATTI_MC_TextureAtlas_GetSprite(atlas, id);
    LIBMATTI_MC_Identifier_Free(id);
    if (sprite == NULL || sprite->contents == NULL)
        return 0;

    int atlasWidth = LIBMATTI_MC_TextureAtlas_GetWidth(atlas);
    int atlasHeight = LIBMATTI_MC_TextureAtlas_GetHeight(atlas);
    if (atlasWidth <= 0 || atlasHeight <= 0)
        return 0;

    // Java: getU0/getV0/getU1/getV1 = (x + inset) / atlasSize - the port
    // keeps the un-inset rect (the vanilla sprites carry no frame inset).
    uvRect[0] = (float) sprite->x / (float) atlasWidth;
    uvRect[1] = (float) sprite->y / (float) atlasHeight;
    uvRect[2] = (float) (sprite->x + sprite->width) / (float) atlasWidth;
    uvRect[3] = (float) (sprite->y + sprite->height) / (float) atlasHeight;
    return 1;
}

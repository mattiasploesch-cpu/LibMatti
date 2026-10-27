// HUD harness: the P5.5 core - the Gui layout unit (the crosshair/hotbar
// geometry out of Gui.java), the POSITION_COLOR_TEXTURE quad packing (the
// two-triangle blit the GuiRenderer batches) and the hotbar item palette the
// client builds from the vanilla block items.

#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/client/gui/GuiLayout.h"
#include "libmatti/net/minecraft/client/gui/GuiRenderer.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockTextures.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"
#include "libmatti/net/minecraft/world/item/Item.h"
#include "libmatti/net/minecraft/world/item/ItemStack.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

static void check_rect(int x, int y, int w, int h, int ex, int ey, int ew, int eh, const char *what)
{
    checks++;
    if (x != ex || y != ey || w != ew || h != eh)
    {
        failures++;
        printf("FAIL: %s (got %d,%d %dx%d, want %d,%d %dx%d)\n", what, x, y, w, h, ex, ey, ew, eh);
    }
}

static void test_crosshair(void)
{
    // Java: (guiWidth - 15) / 2, (guiHeight - 15) / 2 over the 854x480 layout.
    int x, y, w, h;
    LIBMATTI_MC_GuiLayout_CrosshairRect(854, 480, &x, &y, &w, &h);
    check_rect(x, y, w, h, 419, 232, 15, 15, "crosshair rect");

    // odd widths floor the centre (the Java int division).
    LIBMATTI_MC_GuiLayout_CrosshairRect(855, 481, &x, &y, &w, &h);
    check_rect(x, y, w, h, 420, 233, 15, 15, "crosshair rect odd");
}

static void test_hotbar(void)
{
    int x, y, w, h;
    LIBMATTI_MC_GuiLayout_HotbarRect(854, 480, &x, &y, &w, &h);
    check_rect(x, y, w, h, 336, 458, 182, 22, "hotbar rect");

    // Java: i - 91 - 1 + slot * 20, h - 23 over the 24x23 selection sprite.
    LIBMATTI_MC_GuiLayout_HotbarSelectionRect(854, 480, 0, &x, &y, &w, &h);
    check_rect(x, y, w, h, 335, 457, 24, 23, "selection slot 0");
    LIBMATTI_MC_GuiLayout_HotbarSelectionRect(854, 480, 8, &x, &y, &w, &h);
    check_rect(x, y, w, h, 495, 457, 24, 23, "selection slot 8");

    // the selectedSlot clamps into the 0..8 the vanilla hotbar indexes.
    LIBMATTI_MC_GuiLayout_HotbarSelectionRect(854, 480, -3, &x, &y, &w, &h);
    check_rect(x, y, w, h, 335, 457, 24, 23, "selection clamps low");
    LIBMATTI_MC_GuiLayout_HotbarSelectionRect(854, 480, 12, &x, &y, &w, &h);
    check_rect(x, y, w, h, 495, 457, 24, 23, "selection clamps high");

    // Java: renderSlot - 16x16 at (i - 90 + slot*20 + 2, h - 19).
    LIBMATTI_MC_GuiLayout_HotbarSlotRect(854, 480, 0, &x, &y, &w, &h);
    check_rect(x, y, w, h, 339, 461, 16, 16, "slot rect 0");
    LIBMATTI_MC_GuiLayout_HotbarSlotRect(854, 480, 4, &x, &y, &w, &h);
    check_rect(x, y, w, h, 419, 461, 16, 16, "slot rect 4");
    LIBMATTI_MC_GuiLayout_HotbarSlotRect(854, 480, 8, &x, &y, &w, &h);
    check_rect(x, y, w, h, 499, 461, 16, 16, "slot rect 8");
}

static void test_selected_name(void)
{
    // Java: (w - textWidth) / 2 over the h - 59 + 14 creative lift.
    int y = 0;
    int x = LIBMATTI_MC_GuiLayout_SelectedItemNameRect(854, 480, 100, &y);
    check(x == 377, "name centres (w - width) / 2");
    check(y == 435, "name y = h - 59 + 14 (creative)");

    // a wider-than-screen name clamps to the left edge (the Java division).
    x = LIBMATTI_MC_GuiLayout_SelectedItemNameRect(854, 480, 1000, &y);
    check(x == -73, "name x goes negative for wide text");
}

static void test_pack_quad(void)
{
    // The two-triangle pack (v0 v1 v2 + v0 v2 v3) over a 10x6 rect with the
    // ARGB tint 0x80FF4020 folded into the vertex colours.
    LIBMATTI_MC_GuiVertex v[6];
    LIBMATTI_MC_GuiRenderer_PackQuad(v, 5.0f, 7.0f, 10.0f, 6.0f, 0.25f, 0.5f, 0.75f, 1.0f, 0x80FF4020u);

    float eps = 0.0001f;
    check(fabsf(v[0].x - 5.0f) < eps && fabsf(v[0].y - 7.0f) < eps, "v0 = top-left");
    check(fabsf(v[1].x - 5.0f) < eps && fabsf(v[1].y - 13.0f) < eps, "v1 = bottom-left");
    check(fabsf(v[2].x - 15.0f) < eps && fabsf(v[2].y - 13.0f) < eps, "v2 = bottom-right");
    check(fabsf(v[3].x - 5.0f) < eps && fabsf(v[3].y - 7.0f) < eps, "v3 = v0");
    check(fabsf(v[4].x - 15.0f) < eps && fabsf(v[4].y - 13.0f) < eps, "v4 = v2");
    check(fabsf(v[5].x - 15.0f) < eps && fabsf(v[5].y - 7.0f) < eps, "v5 = top-right");

    check(fabsf(v[0].u - 0.25f) < eps && fabsf(v[0].v - 0.5f) < eps, "v0 uv");
    check(fabsf(v[2].u - 0.75f) < eps && fabsf(v[2].v - 1.0f) < eps, "v2 uv");
    check(fabsf(v[5].u - 0.75f) < eps && fabsf(v[5].v - 0.5f) < eps, "v5 uv");

    // Java: the packed ARGB folds as (a, r, g, b) vertex floats.
    check(fabsf(v[0].r - 1.0f) < eps, "red channel");
    check(fabsf(v[0].g - 64.0f / 255.0f) < eps, "green channel");
    check(fabsf(v[0].b - 32.0f / 255.0f) < eps, "blue channel");
    check(fabsf(v[0].a - 128.0f / 255.0f) < eps, "alpha channel");
    for (int i = 1; i < 6; i++)
        check(memcmp(&v[0].r, &v[i].r, 4 * sizeof(float)) == 0, "tint uniform over the quad");
}

static void test_hotbar_palette(void)
{
    // Java: the creative hotbar defaults - 9 stacks over the vanilla block
    // items, count 64, the BlockItem backlink carries the placed block.
    static const char *const names[9] = {
        "STONE", "DIRT", "COBBLESTONE", "OAK_PLANKS", "GLASS",
        "BRICKS", "SAND", "GRAVEL", "OAK_LOG"};
    for (int slot = 0; slot < 9; slot++)
    {
        LIBMATTI_MC_Item *item = LIBMATTI_MC_VanillaItems_GetByName(names[slot]);
        check(item != NULL, "vanilla block item exists");
        if (item == NULL)
            continue;
        check(item->block != NULL, "item carries the BlockItem backlink");

        LIBMATTI_MC_ItemStack *stack = LIBMATTI_MC_ItemStack_NewWithCount(item, 64);
        check(stack != NULL, "stack builds");
        check(!LIBMATTI_MC_ItemStack_IsEmpty(stack), "stack not empty");
        check(LIBMATTI_MC_ItemStack_GetCount(stack) == 64, "stack count 64");
        check(LIBMATTI_MC_ItemStack_GetItem(stack) == item, "stack item roundtrip");
        LIBMATTI_MC_ItemStack_Free(stack);
    }

    // the descriptionId doubles as the name line text (the port has no
    // Component hover names yet) - the block items resolve it at construction
    // (VanillaItems: the prototype map rides the id) so it is non-empty.
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    const char *stoneId = stone != NULL ? LIBMATTI_MC_Item_GetDescriptionId(stone) : NULL;
    check(stoneId != NULL && stoneId[0] != '\0', "stone descriptionId non-empty");
}

static void test_hotbar_atlas(void)
{
    // Java: MODEL_ATLAS - the atlas carries a real sprite for every hotbar
    // block item (the item cells sample the block texture like the game's
    // block-model items) plus the missing-no fallback.
    LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_VanillaBlockTextures_Bootstrap(1024);
    check(atlas != NULL, "atlas bootstraps");
    if (atlas == NULL)
        return;

    static const char *const spritePaths[9] = {
        "block/stone", "block/dirt", "block/cobblestone", "block/oak_planks", "block/glass",
        "block/bricks", "block/sand", "block/gravel", "block/oak_log"};
    static const char *const itemNames[9] = {
        "STONE", "DIRT", "COBBLESTONE", "OAK_PLANKS", "GLASS",
        "BRICKS", "SAND", "GRAVEL", "OAK_LOG"};
    for (int slot = 0; slot < 9; slot++)
    {
        float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        int resolved = LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, spritePaths[slot], uv);
        check(resolved, "hotbar block sprite in atlas");
        if (!resolved)
            continue;
        // a real sprite is a 16px cell on the 160x16 page (0.1 wide), never
        // the full-atlas rect (the smear the missing fallback replaced).
        check(uv[2] - uv[0] > 0.05f && uv[2] - uv[0] < 0.15f, "sprite rect is one cell wide");
        check(uv[3] - uv[1] == 1.0f, "sprite spans the 16px page height");

        // the item's block key maps onto the sprite path (the textureId the
        // HUD builds from the BlockItem backlink).
        LIBMATTI_MC_Item *item = LIBMATTI_MC_VanillaItems_GetByName(itemNames[slot]);
        check(item != NULL && item->block != NULL, "item for sprite path");
        if (item == NULL || item->block == NULL)
            continue;
        LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_Block_GetKey(item->block);
        check(key != NULL, "block key resolves");
        if (key == NULL)
            continue;
        const char *path = LIBMATTI_MC_Identifier_GetPath(key->identifier);
        const char *spriteName = spritePaths[slot] + strlen("block/");
        check(strcmp(path, spriteName) == 0, "block key path == sprite name");
    }

    // the missing sprite answers unknown ids (the checkerboard fallback the
    // HUD and the terrain share).
    float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    check(LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "block/unknown", uv), "unknown id falls back");
    check(uv[2] - uv[0] > 0.05f && uv[2] - uv[0] < 0.15f, "fallback is a cell, not the page");

    LIBMATTI_MC_TextureAtlas_Free(atlas);
}

int main(void)
{
    LIBMATTI_MC_Bootstrap_BootStrap();
    test_crosshair();
    test_hotbar();
    test_selected_name();
    test_pack_quad();
    test_hotbar_palette();
    test_hotbar_atlas();
    printf("hud: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}

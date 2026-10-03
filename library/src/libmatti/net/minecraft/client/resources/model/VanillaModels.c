// The embedded vanilla block models (implementation). The JSONs mirror the
// vanilla asset files 1:1 (cube_all, cube, stone, dirt).

#include "libmatti/net/minecraft/client/resources/model/VanillaModels.h"

#include <stdlib.h>
#include <string.h>

// Java: assets/minecraft/models/block/cube_all.json
const char *const LIBMATTI_MC_VanillaModels_CUBE_ALL =
    "{\n"
    "    \"parent\": \"block/block\",\n"
    "    \"render_type\": \"minecraft:solid\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"#missing\",\n"
    "        \"particle\": \"#all\"\n"
    "    },\n"
    "    \"elements\": [\n"
    "        {   \"from\": [ 0, 0, 0 ],\n"
    "            \"to\": [ 16, 16, 16 ],\n"
    "            \"faces\": {\n"
    "                \"down\":  { \"texture\": \"#all\", \"cullface\": \"down\" },\n"
    "                \"up\":    { \"texture\": \"#all\", \"cullface\": \"up\" },\n"
    "                \"north\": { \"texture\": \"#all\", \"cullface\": \"north\" },\n"
    "                \"south\": { \"texture\": \"#all\", \"cullface\": \"south\" },\n"
    "                \"west\":  { \"texture\": \"#all\", \"cullface\": \"west\" },\n"
    "                \"east\":  { \"texture\": \"#all\", \"cullface\": \"east\" }\n"
    "            }\n"
    "        }\n"
    "    ]\n"
    "}\n";

// Java: assets/minecraft/models/block/cube.json
const char *const LIBMATTI_MC_VanillaModels_CUBE =
    "{\n"
    "    \"parent\": \"block/block\",\n"
    "    \"render_type\": \"minecraft:solid\",\n"
    "    \"textures\": {\n"
    "        \"particle\": \"#north\",\n"
    "        \"north\": \"#north\",\n"
    "        \"south\": \"#south\",\n"
    "        \"east\": \"#east\",\n"
    "        \"west\": \"#west\",\n"
    "        \"up\": \"#up\",\n"
    "        \"down\": \"#down\"\n"
    "    },\n"
    "    \"elements\": [\n"
    "        {   \"from\": [ 0, 0, 0 ],\n"
    "            \"to\": [ 16, 16, 16 ],\n"
    "            \"faces\": {\n"
    "                \"down\":  { \"texture\": \"#down\", \"cullface\": \"down\" },\n"
    "                \"up\":    { \"texture\": \"#up\", \"cullface\": \"up\" },\n"
    "                \"north\": { \"texture\": \"#north\", \"cullface\": \"north\" },\n"
    "                \"south\": { \"texture\": \"#south\", \"cullface\": \"south\" },\n"
    "                \"west\":  { \"texture\": \"#west\", \"cullface\": \"west\" },\n"
    "                \"east\":  { \"texture\": \"#east\", \"cullface\": \"east\" }\n"
    "            }\n"
    "        }\n"
    "    ]\n"
    "}\n";

// Java: assets/minecraft/models/block/stone.json
const char *const LIBMATTI_MC_VanillaModels_STONE =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/stone\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/dirt.json
const char *const LIBMATTI_MC_VanillaModels_DIRT =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/dirt\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/cobblestone.json (parent: cube_all)
const char *const LIBMATTI_MC_VanillaModels_COBBLESTONE =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/cobblestone\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/oak_planks.json (parent: cube_all)
const char *const LIBMATTI_MC_VanillaModels_OAK_PLANKS =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/oak_planks\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/glass.json (parent: cube_all)
const char *const LIBMATTI_MC_VanillaModels_GLASS =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/glass\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/bricks.json (parent: cube_all)
const char *const LIBMATTI_MC_VanillaModels_BRICKS =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/bricks\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/sand.json (parent: cube_all)
const char *const LIBMATTI_MC_VanillaModels_SAND =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/sand\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/gravel.json (parent: cube_all)
const char *const LIBMATTI_MC_VanillaModels_GRAVEL =
    "{\n"
    "    \"parent\": \"block/cube_all\",\n"
    "    \"textures\": {\n"
    "        \"all\": \"block/gravel\"\n"
    "    }\n"
    "}\n";

// Java: assets/minecraft/models/block/cube_column.json - the column cube: the
// ends carry "end", the four sides "side".
const char *const LIBMATTI_MC_VanillaModels_CUBE_COLUMN =
    "{\n"
    "    \"parent\": \"block/block\",\n"
    "    \"render_type\": \"minecraft:solid\",\n"
    "    \"textures\": {\n"
    "        \"particle\": \"#side\",\n"
    "        \"down\": \"#end\",\n"
    "        \"up\": \"#end\",\n"
    "        \"north\": \"#side\",\n"
    "        \"south\": \"#side\",\n"
    "        \"west\": \"#side\",\n"
    "        \"east\": \"#side\"\n"
    "    },\n"
    "    \"elements\": [\n"
    "        {   \"from\": [ 0, 0, 0 ],\n"
    "            \"to\": [ 16, 16, 16 ],\n"
    "            \"faces\": {\n"
    "                \"down\":  { \"texture\": \"#down\", \"cullface\": \"down\" },\n"
    "                \"up\":    { \"texture\": \"#up\", \"cullface\": \"up\" },\n"
    "                \"north\": { \"texture\": \"#north\", \"cullface\": \"north\" },\n"
    "                \"south\": { \"texture\": \"#south\", \"cullface\": \"south\" },\n"
    "                \"west\":  { \"texture\": \"#west\", \"cullface\": \"west\" },\n"
    "                \"east\":  { \"texture\": \"#east\", \"cullface\": \"east\" }\n"
    "            }\n"
    "        }\n"
    "    ]\n"
    "}\n";

// Java: assets/minecraft/models/block/oak_log.json (parent: cube_column; the
// demo atlas keeps one bark texture, so end and side ride block/oak_log).
const char *const LIBMATTI_MC_VanillaModels_OAK_LOG =
    "{\n"
    "    \"parent\": \"block/cube_column\",\n"
    "    \"textures\": {\n"
    "        \"end\": \"block/oak_log\",\n"
    "        \"side\": \"block/oak_log\"\n"
    "    }\n"
    "}\n";

const char *const LIBMATTI_MC_VanillaModels_STONE_ID = "minecraft:block/stone";
const char *const LIBMATTI_MC_VanillaModels_DIRT_ID = "minecraft:block/dirt";
const char *const LIBMATTI_MC_VanillaModels_COBBLESTONE_ID = "minecraft:block/cobblestone";
const char *const LIBMATTI_MC_VanillaModels_OAK_PLANKS_ID = "minecraft:block/oak_planks";
const char *const LIBMATTI_MC_VanillaModels_GLASS_ID = "minecraft:block/glass";
const char *const LIBMATTI_MC_VanillaModels_BRICKS_ID = "minecraft:block/bricks";
const char *const LIBMATTI_MC_VanillaModels_SAND_ID = "minecraft:block/sand";
const char *const LIBMATTI_MC_VanillaModels_GRAVEL_ID = "minecraft:block/gravel";
const char *const LIBMATTI_MC_VanillaModels_OAK_LOG_ID = "minecraft:block/oak_log";

const char *LIBMATTI_MC_VanillaModels_ModelById(const char *modelId)
{
    if (modelId == NULL)
        return NULL;
    if (strcmp(modelId, "minecraft:block/cube_all") == 0 || strcmp(modelId, "block/cube_all") == 0)
        return LIBMATTI_MC_VanillaModels_CUBE_ALL;
    if (strcmp(modelId, "minecraft:block/cube") == 0 || strcmp(modelId, "block/cube") == 0)
        return LIBMATTI_MC_VanillaModels_CUBE;
    if (strcmp(modelId, "minecraft:block/stone") == 0 || strcmp(modelId, "block/stone") == 0)
        return LIBMATTI_MC_VanillaModels_STONE;
    if (strcmp(modelId, "minecraft:block/dirt") == 0 || strcmp(modelId, "block/dirt") == 0)
        return LIBMATTI_MC_VanillaModels_DIRT;
    if (strcmp(modelId, "minecraft:block/cobblestone") == 0 || strcmp(modelId, "block/cobblestone") == 0)
        return LIBMATTI_MC_VanillaModels_COBBLESTONE;
    if (strcmp(modelId, "minecraft:block/oak_planks") == 0 || strcmp(modelId, "block/oak_planks") == 0)
        return LIBMATTI_MC_VanillaModels_OAK_PLANKS;
    if (strcmp(modelId, "minecraft:block/glass") == 0 || strcmp(modelId, "block/glass") == 0)
        return LIBMATTI_MC_VanillaModels_GLASS;
    if (strcmp(modelId, "minecraft:block/bricks") == 0 || strcmp(modelId, "block/bricks") == 0)
        return LIBMATTI_MC_VanillaModels_BRICKS;
    if (strcmp(modelId, "minecraft:block/sand") == 0 || strcmp(modelId, "block/sand") == 0)
        return LIBMATTI_MC_VanillaModels_SAND;
    if (strcmp(modelId, "minecraft:block/gravel") == 0 || strcmp(modelId, "block/gravel") == 0)
        return LIBMATTI_MC_VanillaModels_GRAVEL;
    if (strcmp(modelId, "minecraft:block/cube_column") == 0 || strcmp(modelId, "block/cube_column") == 0)
        return LIBMATTI_MC_VanillaModels_CUBE_COLUMN;
    if (strcmp(modelId, "minecraft:block/oak_log") == 0 || strcmp(modelId, "block/oak_log") == 0)
        return LIBMATTI_MC_VanillaModels_OAK_LOG;
    return NULL;
}

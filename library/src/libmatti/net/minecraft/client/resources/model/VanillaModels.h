// The vanilla block models, embedded. Java ships them in
// assets/minecraft/models/block/*.json; the port carries the handful the
// demo level needs (cube_all family + stone/dirt) as C strings so the bake
// runs without a resource pack on disk.

#ifndef MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_VANILLAMODELS_H
#define MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_VANILLAMODELS_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: assets/minecraft/models/block/cube_all.json - the "all"-textured cube.
extern const char *const LIBMATTI_MC_VanillaModels_CUBE_ALL;
// Java: assets/minecraft/models/block/cube.json - the per-face cube.
extern const char *const LIBMATTI_MC_VanillaModels_CUBE;
// Java: assets/minecraft/models/block/stone.json (parent: cube_all)
extern const char *const LIBMATTI_MC_VanillaModels_STONE;
// Java: assets/minecraft/models/block/dirt.json (parent: cube_all)
extern const char *const LIBMATTI_MC_VanillaModels_DIRT;
// The cube_all family slice the demo level renders (the hotbar palette).
extern const char *const LIBMATTI_MC_VanillaModels_COBBLESTONE;
extern const char *const LIBMATTI_MC_VanillaModels_OAK_PLANKS;
extern const char *const LIBMATTI_MC_VanillaModels_GLASS;
extern const char *const LIBMATTI_MC_VanillaModels_BRICKS;
extern const char *const LIBMATTI_MC_VanillaModels_SAND;
extern const char *const LIBMATTI_MC_VanillaModels_GRAVEL;
// Java: assets/minecraft/models/block/cube_column.json (the log parent)
extern const char *const LIBMATTI_MC_VanillaModels_CUBE_COLUMN;
// Java: assets/minecraft/models/block/oak_log.json (parent: cube_column)
extern const char *const LIBMATTI_MC_VanillaModels_OAK_LOG;

// Java: ModelBakery's model resource ids ("minecraft:block/stone").
extern const char *const LIBMATTI_MC_VanillaModels_STONE_ID;
extern const char *const LIBMATTI_MC_VanillaModels_DIRT_ID;
extern const char *const LIBMATTI_MC_VanillaModels_COBBLESTONE_ID;
extern const char *const LIBMATTI_MC_VanillaModels_OAK_PLANKS_ID;
extern const char *const LIBMATTI_MC_VanillaModels_GLASS_ID;
extern const char *const LIBMATTI_MC_VanillaModels_BRICKS_ID;
extern const char *const LIBMATTI_MC_VanillaModels_SAND_ID;
extern const char *const LIBMATTI_MC_VanillaModels_GRAVEL_ID;
extern const char *const LIBMATTI_MC_VanillaModels_OAK_LOG_ID;

// Java: ModelDiscovery's parent resolution - the embedded parent models by id
// ("minecraft:block/cube_all" etc.), NULL when unknown.
const char *LIBMATTI_MC_VanillaModels_ModelById(const char *modelId);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RESOURCES_MODEL_VANILLAMODELS_H

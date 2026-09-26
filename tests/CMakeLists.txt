# Matticraft tests - the harnesses that were previously throwaway /tmp builds.
#
# This file is part of the root project: it adds no second project and no own
# build tree. Everything lands in the normal build folder, so one run of
# "matticraft" (which depends on every harness) builds and runs it all.
#
# The standalone build still works:
#
#     cmake -S tests -B tests/build
#     cmake --build tests/build
#     ctest --test-dir tests/build --output-on-failure
#
# tests/README.md documents each harness.

get_filename_component(MATTI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

set(MATTI_LIBRARY_DIR "${MATTI_ROOT}/library/src" CACHE PATH "include root of libmatti")
set(MATTI_LIBMATTI_LIB "" CACHE PATH "the built libmatti (standalone build only)")
set(MATTI_MODS_DIR "${MATTI_ROOT}/mods" CACHE PATH "output folder for built mods")

if(NOT TARGET matti)
    # Standalone build (cmake -S tests): the library comes in as the prebuilt
    # static archive. Inside the root project the matti target already exists.
    if(NOT MATTI_LIBMATTI_LIB)
        message(FATAL_ERROR "Standalone test build needs MATTI_LIBMATTI_LIB (build the root project once first)")
    endif()
    add_library(matti_imported STATIC IMPORTED)
    set_target_properties(matti_imported PROPERTIES IMPORTED_LOCATION "${MATTI_LIBMATTI_LIB}")
    add_library(matti ALIAS matti_imported)
endif()

include("${MATTI_ROOT}/mods/cmake/MattiMod.cmake")

# Every harness links the same way: headers of library/src, the matti target.
macro(matti_test target)
    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE "${MATTI_LIBRARY_DIR}")
    target_link_libraries(${target} PRIVATE matti dl)
    target_compile_options(${target} PRIVATE -g)
endmacro()

# ---------------------------------------------------------------------------
# nbt: SNBT parser/writer + NbtIo binary/gzip round trips
# ---------------------------------------------------------------------------

matti_test(test_nbt nbt/NbtHarness.c)
add_test(NAME nbt COMMAND test_nbt)

# ---------------------------------------------------------------------------
# math: Mth, Vec3, BlockPos, ChunkPos, AABB, Direction
# ---------------------------------------------------------------------------

matti_test(test_math math/MathHarness.c)
add_test(NAME math COMMAND test_math)

# ---------------------------------------------------------------------------
# blockstate: Property/StateHolder/StateDefinition/BlockState machinery
# ---------------------------------------------------------------------------

matti_test(test_blockstate blockstate/BlockStateHarness.c)
add_test(NAME blockstate COMMAND test_blockstate)

# ---------------------------------------------------------------------------
# vanilla: Bootstrap.bootStrap() fills BLOCK/ITEM with the generated vanilla data
# ---------------------------------------------------------------------------

matti_test(test_vanilla vanilla/VanillaHarness.c)
add_test(NAME vanilla COMMAND test_vanilla)

# ---------------------------------------------------------------------------
# itemstack: ItemStack components/count mechanics over the Item prototype
# ---------------------------------------------------------------------------

matti_test(test_itemstack itemstack/ItemStackHarness.c)
add_test(NAME itemstack COMMAND test_itemstack)

# ---------------------------------------------------------------------------
# level: in-memory Level/LevelChunk/LevelChunkSection write-read path
# ---------------------------------------------------------------------------

matti_test(test_level level/LevelHarness.c)
add_test(NAME level COMMAND test_level)

# entity: the P5.1 core - EntityType registry, Entity base (pos/rot/bbox, tags,
# NBT round trip), vanilla entity constants, Level add/remove/query
matti_test(test_entity entity/EntityHarness.c)
add_test(NAME entity COMMAND test_entity)

# player: the P5.2 core - KeyMapping registry (bind/isDown/click), the Input
# record + move vector (KeyboardInput), the Player entity (abilities/food/NBT)
# and the LocalPlayer wiring
matti_test(test_player player/PlayerHarness.c)
add_test(NAME player COMMAND test_player)

# physics: the P5.3 core - the block-collision scan (AABB vs BlockStates),
# noBlockCollision, the per-axis collide sweep and the Entity move path
matti_test(test_physics physics/PhysicsHarness.c)
add_test(NAME physics COMMAND test_physics)

# pick: the P5.4 core - the level ray cast (BlockGetter.clip over the
# traverseBlocks DDA), the per-cell cube clip with the entry face, the reach
# cutoff and the ClipContext shape gate
matti_test(test_pick pick/PickHarness.c)
add_test(NAME pick COMMAND test_pick)

# hud: the P5.5 core - the Gui layout unit (crosshair/hotbar rects out of
# Gui.java), the POSITION_COLOR_TEXTURE quad packing and the hotbar item
# palette the client drives
matti_test(test_hud hud/HudHarness.c)
add_test(NAME hud COMMAND test_hud)

# sound: the P5.6 core - the SoundEngine stack over the LIBMATTI_OAL_*
# wrapper (library/channel/boot lifecycle, headless no-op tolerance) and
# the SoundEvents resolve cache the SoundTypes ride
matti_test(test_sound sound/SoundHarness.c)
add_test(NAME sound COMMAND test_sound)

# ao: the P4.4 AO kernel over a real level (BlockColors providers, shade table,
# the tesselate paths through the dispatcher)
matti_test(test_ao ao/AoHarness.c)
add_test(NAME ao COMMAND test_ao)

# ---------------------------------------------------------------------------
# gl: GL binding constants, dlopen availability and no-driver fallbacks
# ---------------------------------------------------------------------------

matti_test(test_gl gl/GLHarness.c)
add_test(NAME gl COMMAND test_gl)

# ---------------------------------------------------------------------------
# window: DeltaTracker cadence + headless Minecraft skeleton entry
# ---------------------------------------------------------------------------

matti_test(test_window window/WindowHarness.c)
target_link_libraries(test_window PRIVATE pthread)
add_test(NAME window COMMAND test_window)

# ---------------------------------------------------------------------------
# rendersystem: state cache, projection/model-view stack, index buffers,
# vertex formats and the VAO cache (no GL context needed)
# ---------------------------------------------------------------------------

matti_test(test_rendersystem rendersystem/RenderSystemHarness.c)
add_test(NAME rendersystem COMMAND test_rendersystem)

# ---------------------------------------------------------------------------
# bufferbuilder: ByteBufferBuilder reserve/build, vertex layouts, element
# skipping, the NEW_ENTITY fast path, quad sorting and the Tesselator
# ---------------------------------------------------------------------------

matti_test(test_bufferbuilder bufferbuilder/BufferBuilderHarness.c)
add_test(NAME bufferbuilder COMMAND test_bufferbuilder)

# ---------------------------------------------------------------------------
# font: SimpleFont pack path over the real Monocraft.ttf (glyph metrics,
# stringWidth/measureText, the no-GL draw fallback)
# ---------------------------------------------------------------------------

matti_test(test_font font/FontHarness.c)
add_test(NAME font COMMAND test_font
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------
# shader: GLSL preprocessor, ShaderManager source/compile paths and the
# GlProgram/Uniform model (no GL context needed)
# ---------------------------------------------------------------------------

matti_test(test_shader shader/ShaderHarness.c)
target_include_directories(test_shader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME shader COMMAND test_shader
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------
# texture: NativeImage PNG decode, metadata, Stitcher layout, SpriteLoader
# and TextureAtlas over a fixture pack (no GL context needed)
# ---------------------------------------------------------------------------

matti_test(test_texture texture/TextureHarness.c)
add_test(NAME texture COMMAND test_texture
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------
# brigadier: dispatcher parse/execute/usage over the ported command tree
# ---------------------------------------------------------------------------

matti_test(test_brigadier brigadier/BrigadierHarness.c)
add_test(NAME brigadier COMMAND test_brigadier)

# ---------------------------------------------------------------------------
# chunkmesh: the P4.1 section compile pipeline over a real level
# ---------------------------------------------------------------------------

matti_test(test_chunkmesh chunkmesh/ChunkMeshHarness.c)
add_test(NAME chunkmesh COMMAND test_chunkmesh)

# ---------------------------------------------------------------------------
# atlastextures: the P4.3 atlas bootstrap + model bake with real sprite rects
# ---------------------------------------------------------------------------

matti_test(test_atlastextures atlastextures/AtlasTexturesHarness.c)
add_test(NAME atlastextures COMMAND test_atlastextures)

# ---------------------------------------------------------------------------
# frustum: the P4.3 plane extraction, cube tests and camera contract
# ---------------------------------------------------------------------------

matti_test(test_frustum frustum/FrustumHarness.c)
add_test(NAME frustum COMMAND test_frustum)

# ---------------------------------------------------------------------------
# modscan: class-file scan harness
# ---------------------------------------------------------------------------

matti_test(test_modscan modscan/ModScanHarness.c)
add_test(NAME modscan COMMAND test_modscan
        "${CMAKE_CURRENT_SOURCE_DIR}/modscan/out/example/ExampleMod.class"
        "${CMAKE_CURRENT_SOURCE_DIR}/modscan/out/example/ExampleSubscriber.class")

# so2scan: reads a real mod .so through JarContents and scans its entries
matti_test(test_soscan modscan/SoScanHarness.c)
add_dependencies(test_soscan fixture_classmod)
add_test(NAME soscan COMMAND test_soscan
        "$<TARGET_FILE:fixture_classmod>")

# ---------------------------------------------------------------------------
# mixinfacade: config registration and error branches
# ---------------------------------------------------------------------------

matti_test(test_mixinfacade mixinfacade/MixinFacadeHarness.c)
add_dependencies(test_mixinfacade fixture_mixina fixture_mixinb)
add_test(NAME mixinfacade COMMAND test_mixinfacade
        "$<TARGET_FILE:fixture_mixina>"
        "$<TARGET_FILE:fixture_mixinb>")

# The two fixture mods. Their sources are empty on purpose; manifest and
# neoforge.mods.toml are embedded sections, the mixins.json is appended with
# objcopy because the entry name is not an ELF-safe path.
matti_add_mod(fixture_mixina
        MOD_ID mixina MOD_VERSION 1.0
        SOURCES mixinfacade/MixinaMod.c
        RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/mixinfacade/mixina-res"
        OUTPUT_NAME "mixina-fixture" OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
)
matti_add_section(fixture_mixina "mixina.mixins.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/mixinfacade/mixina.mixins.json")

matti_add_mod(fixture_mixinb
        MOD_ID mixinb MOD_VERSION 1.0
        SOURCES mixinfacade/MixinbMod.c
        RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/mixinfacade/mixinb-res"
        OUTPUT_NAME "mixinb-fixture" OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
)

# ---------------------------------------------------------------------------
# mixin/m4proof: the M4 hook table + M2 GOT patch proof (non-PIE on purpose)
# ---------------------------------------------------------------------------

matti_test(test_m4proof mixin/m4proof/M4Proof.c)
target_compile_options(test_m4proof PRIVATE -no-pie -rdynamic)
target_link_options(test_m4proof PRIVATE -no-pie -rdynamic)
add_dependencies(test_m4proof fixture_classmod)
add_test(NAME m4proof COMMAND test_m4proof
        "$<TARGET_FILE:fixture_classmod>")

# The mod fixture both the M4 harness and the .so scan harness run against:
# one MATTI_MIXIN hook into the harness's "matticraft::demo::tick" target and
# one "example/ExampleMod.class" entry (appended with objcopy), so the scan
# harness sees a class file inside the .so exactly like the real mods have.
matti_add_mod(fixture_classmod
        MOD_ID classmod MOD_VERSION 1.0
        SOURCES mixin/m4proof/ClassMod.c mixin/m4proof/classmod.mixins.c
        OUTPUT_NAME "classmod-fixture" OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
)
matti_add_section(fixture_classmod "example/ExampleMod.class"
        "${CMAKE_CURRENT_SOURCE_DIR}/modscan/out/example/ExampleMod.class")

# The standalone output folder the old docs pointed at; the harnesses take the
# fixture .so by argument, so where it lands does not matter.
set(MATTI_TESTS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "unused, kept for compatibility")

# ---------------------------------------------------------------------------
# Every harness finishes in seconds. A hung test (a GL stack stuck on a broken
# display, a stalled fixture) must fail the suite instead of blocking CI
# forever, so every test gets a hard timeout.
# ---------------------------------------------------------------------------

get_property(MATTI_ALL_TESTS DIRECTORY PROPERTY TESTS)
if(MATTI_ALL_TESTS)
    set_tests_properties(${MATTI_ALL_TESTS} PROPERTIES TIMEOUT 30)
endif()

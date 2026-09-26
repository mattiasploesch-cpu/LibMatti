    # Plan: Minecraft-Port (net.minecraft.*) in Phasen

> **Status-Marker:** ✅ fertig · 🟨 teilweise · ⬜ offen
> Stand: 23.09.2026 — Build grün, ctest 22/22, `runClient` bootet FML vollständig (Mods, Configs, Mixins, Registries, Vanilla-Bootstrap), rendert die Welt-Ladeansicht mit Titel, Progress-Bar und gebackenem Terrain, cullt Sections per Frustum (Camera + FrustumIntersection 1:1 JOML) und rendert über den BlockRenderDispatcher mit AO + Tinting; Stable-Release-Pipeline (AppImage, Portable-Zip, Dev-SDK) per `workflow_dispatch` verfügbar.
> Dieser Plan ist die bereinigte Übersicht; inhaltlich wurde nichts umgeschrieben, nur der Fortschritt markiert.

**Orientierung:** unten nach oben bauen — jede Phase hat ein sichtbares Ziel in `runClient`, damit du immer was siehst.

────────────────────────────────────────────────────────────────────────────────

## P0 — Fundament-Bibliotheken (MC setzt sie überall voraus)

| #  | Was                                                                                                     | Anmerkung                                                          | Status |
|----|---------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------|--------|
| 1  | ResourceLocation/Identifier + Parser (regex-los, `:` + Namespace-Check)                                 | Das Absolute Erste. Hängt nichts, aber alles andere hängt daran    | ✅     |
| 2  | JOML (Vector3f/4f, Matrix4f, Quaternionf, AxisAngle)                                                    | vendor/JOML liegt schon geklont → 1:1 portieren                    | ✅     |
| 3  | java.util-Fundament (generische Map/List/Set mit Comparators, Optional, Random, …)                       | Basis war da, braucht Vollständigkeit                              | 🟨     |
| 4  | java.util.function (Supplier/Function/BiFunction/Predicate/Consumer-Baukasten)                           | MC-Datenstrukturen sind voll davon                                 | 🟨     |
| 5  | java.nio Buffer (ByteBuffer/ByteOrder)                                                                  | für NBT, Netty, Bilder                                             | ✅     |
| 6  | java.util.regex (Pattern/Matcher-Minimum)                                                               | Chat, Resource-Pfade                                               | ✅     |
| 7  | Guava-Ersatz (ImmutableList/Map/Set, Lists, Maps, BiMap, Table, Joiner/Splitter)                         | MC nutzt Guava massiv                                              | 🟨     |
| 8  | fastutil-Ersatz (Int2ObjectMap, Object2IntMap, LongOpenHashSet — nur die tatsächlich genutzten)          | Chunks/Entities brauchen die                                       | 🟨     |
| 9  | java.util.concurrent (CompletableFuture, ExecutorService, Locks)                                         | Parallel-Dispatch läuft dann echt                                  | 🟨     |

**Ziel:** kompiliert, keine Game-Optik, aber ohne das geht nichts. — **erreicht.**

Vor P0 fertig (frühere Meilensteine, nicht Teil des Plans):
- **BSL** (BootstrapLauncher), **SJH** (SecureJarHandler komplett), **ModLauncher**, **FML** (Discovery, LanguageProvider, EventBus, ConfigTracker, ModContainer/ModList, `.so`-Mods, M4-Hook-Tabelle + M2-GOT-Patcher, Mixin-Bootstrap), **neoforgespi**, `client/main.c`-Handover, GLFW-Port, earlydisplay-Fenster, TOML/JSON/Gson-Reader, Zip/Inflater.

────────────────────────────────────────────────────────────────────────────────

## P1 — Registry- & Resource-Kern (verbindet MC an deine FML-Arbeit)

| # | Was                                                                                                    | Status | Anmerkung |
|---|--------------------------------------------------------------------------------------------------------|--------|-----------|
| 1 | ResourceKey + BuiltInRegistries (MappedRegistry, DefaultedMappedRegistry, WritableRegistry, `Registry.register`) | ✅ | an GameData/RegistryManager/RegisterEvent angebunden — `RegisterEvent.register` schreibt echt in die Registry |
| 2 | NBT (Tag-Hierarchie, CompoundTag/ListTag, SNBT-Parser/Writer, NbtIo binär+gzip)                          | ✅ | eigener Test-Harness |
| 3 | Resource/Pack-System (PackResources, PackRepository, ResourceManager)                                    | ✅ | `server/packs/*` existiert, hängt am ResourcePackLoader-Port |
| 4 | Brigadier (CommandTree, ArgumentTypes, Dispatcher, Suggestions)                                          | ✅ | eigener Test-Harness |
| 5 | vanilla Bootstrap (alle Vanilla-Blocks/Items als Daten über RegisterEvent)                               | ✅ | 1142 Blocks + 1481 Items generiert (VanillaBlocks/VanillaItems), Example-Mod registriert eigene Blöcke daneben |

**Ziel:** ein Mod kann über DeferredRegister einen Block registrieren und du siehst ihn im Registry-Dump. — **erreicht** (Example-Mod registriert `examplemod:example_block` + `example_item` über RegisterEvent neben den Vanilla-Daten).

────────────────────────────────────────────────────────────────────────────────

## P2 — Domänen-Kern (die „Welt“ als Datenmodell)

| # | Was                                                                    | Status | Anmerkung |
|---|------------------------------------------------------------------------|--------|-----------|
| 1 | math (Vec3, BlockPos, ChunkPos, AABB, Mth, Direction)                   | ✅     | inkl. Vec3i-Basis, eigener Test-Harness |
| 2 | BlockState/StateDefinition (Property-System, State-Tabelle)             | ✅     | Property/Boolean/Integer/Enum, StateHolder mit neighbours, kartesisches Produkt 1:1 |
| 3 | BlockBehaviour/Blocks (Registry-Objekte mit Properties)                 | ✅     | Properties komplett (alle Defaults + Builder), MapColor/SoundType/PushReaction/NoteBlockInstrument, BlockStateProperties-Konstanten |
| 4 | Items/ItemStack + DataComponents (1.21-Komponenten-System)              | ✅     | DataComponentType/Map/Patch/PatchedMap, 88 DataComponents-Konstanten, Item.Properties (stacksTo/durability/rarity/tool/armor-Helper), Rarity/ToolMaterial/ArmorMaterials/ArmorType, VanillaItems generiert mit echten Chains, TagKey-Port |
| 5 | Level/LevelReader/BlockGetter-Hierarchie (in-memory Welt, kein Speichern) | ✅   | LevelHeightAccessor, BlockGetter, LevelReader, Level mit Chunk-Map + gameTime/dayTime, Bounds (VOID_AIR) |
| 6 | Chunk/LevelChunk (Datenlayout, BlockEntity-Container)                   | ✅     | ChunkAccess-Basis (sections, heightmaps, postProcessing, unsaved/inhabitedTime), LevelChunkSection (4096 States + Counts), LevelChunk mit EntityCreationType/promotePending/removeBlockEntity, Heightmap (6 Types, prime/update/setRawData), BlockEntity + 49 BlockEntityTypes generiert |

**Ziel:** `runClient` kann eine Welt-Instanz im Speicher anlegen, Blöcke setzen/lesen. — **erreicht** (Level-Harness: 36 Checks, setBlock/getBlockState über Chunks, Heightmaps, BlockEntity-Lifecycle).

────────────────────────────────────────────────────────────────────────────────

## P3 — Fenster & Rendering-Basis (der erste sichtbare Durchbruch)

| # | Was                                                                              | Status | Anmerkung |
|---|----------------------------------------------------------------------------------|--------|-----------|
| 1 | OpenGL-Binding (LWJGL-GL11-Port über dlopen, wie der GLFW-Port)                   | ✅     | org/lwjgl/opengl: 112 Funktionen (GL11–GL33, DSA, KHRDebug, Queries), 123 GlConst-Konstanten, dlopen libGL/EGL/OSMesa + glXGetProcAddress/eglGetProcAddress-Kette; test_gl 44 Checks grün |
| 2 | Window/GameLoop-Gerüst — echter `Minecraft`-Klassen-Ersatz: init → while(!shouldClose) tick+render → Shutdown | ✅ | `net/minecraft/client/Minecraft` + `DeltaTracker` + `main/GameConfig`; Run-Loop feuert Mod-Mixins pro Tick; stopperThread beendet sauber |
| 3 | RenderSystem + GLState (State-Stack, VAO/VBO)                                     | ✅     | `blaze3d/systems/RenderSystem` (Thread-Asserts, flipFrame, limitDisplayFPS, ModelView-Stack, AutoStorageIndexBuffer), `blaze3d/opengl/GlStateManager` (State-Cache: Depth/Blend/Cull/PolyOffset/LogicOp/Scissor/Texture/FBO), `blaze3d/buffers/GpuBuffer+GlBuffer`, `blaze3d/vertex/VertexFormat+VertexFormatElement`, `VertexArrayCache` (Emulated+Separate inkl. Mesa-Workaround); test_rendersystem grün |
| 4 | BufferBuilder/MeshData + VertexFormat                                             | ✅     | `ByteBufferBuilder` (reserve/build/discard mit Java-Generation-Schema), `BufferBuilder` (Element-Writer mit beginElement-Skip, LINES-Duplizierung, NEW_ENTITY/BLOCK-Fast-Path), `MeshData` (DrawState/SortState, sortQuads über CompactVectorArray + VertexSorting), `VertexFormat`-Builder (Namen, offsetsByElement[32], ElementsMask, uploadImmediate), `DefaultVertexFormat` (alle 15 Vanilla-Formate), `Tesselator` (786432-Bytes-Singleton); test_bufferbuilder 49 Checks grün |
| 5 | Shader-System (Core-Shader, GLSL-Dateien laden)                                   | ✅     | `GlslPreprocessor` (moj_import-Expansion, #line-Emission, Version-Lifting, injectDefines), `ShaderType` (.vsh/.fsh), `GlShaderModule` (+INVALID_SHADER-Singleton), `Uniform` (Sampler/Ubo/Utb), `GlProgram` (link mit Attrib-Binding, setupUniforms inkl. Built-ins Projection/Lighting/Fog/Globals), `ShaderManager` (GLSL über ResourceManager, Compile-Cache); test_shader grün |
| 6 | TextureManager + TextureAtlas — PNG-Dekodierung über den ImageIO-Port             | ✅     | `NativeImage`+`ARGB`+`TextureUtil` (PNG über PngReader-Port), `TextureMetadataSection`/`AnimationMetadataSection`, `AbstractTexture`+`SimpleTexture`+`DynamicTexture`, `TextureManager` (Missing-Texture-Registrierung, Minecraft-Konstruktor wired), `Stitcher` (Java-Region-Algorithmus 1:1), `SpriteContents`+`TextureAtlasSprite`+`SpriteLoader`+`MissingTextureAtlasSprite`, `SpriteSourceList` (directory/single_file/filter über alle Namespaces), `TextureAtlas` (Upload 1:1); test_texture 11 Checks grün, runClient ruft TextureManager_Tick pro Frame |
| 7 | Font-Rendering (Monocraft.ttf liegt in den FML-Theme-Resourcen; stb_truetype wie earlydisplay) | ✅ | `vendor/stb` (stb_truetype-Upstream, 1 TU instanziiert), `org/lwjgl/stb/STBTruetype` (InitFont/Pack*/GetPackedQuad wie LWJGL), `earlydisplay/SimpleFont` (Pack-Pfad 1:1: Size 24, ASCII 32–126, 256×128 GL_R8/GL_RED-Seite, Glyph-Quads), `earlydisplay/FontShader` (gui.vert + gui_font.frag eingebettet, screenSize-Uniform), Minecraft zeichnet „Matticraft <Version>" mit Shadow oben links; glUniform2f/4f + glGetShaderiv/Programiv ins GL-Binding ergänzt; test_font 11 Checks grün |

**Ziel:** `runClient` zeigt ein Fenster mit Clear-Color + Text „Matticraft 1.21.11“.

**Checkup nach P3 (erledigt):** ctest 18/18 grün; runClient-Boot-Sequenz fehlerfrei (GL device erkannt, Configs geladen, Mixins feuern, Stop→Destroy sauber). Kernfund gefixt: `MATTI_SOURCE_DIR` hing nur am Client-Target, nicht am `matti`-Target — im echten Client war deshalb `font == NULL` und `fontProgram == 0` (Titel still deaktiviert). Nach dem Fix: Font geladen, Shader kompiliert, `vertexCount = 68` („Matticraft 1.21.11“, 17 Glyphen × 4) mit echter GL-Textur im Draw. SimpleFont/New-Fehlerpfade melden jetzt sichtbar auf stderr statt still NULL zu liefern. TODO-Scan über den P3-Bereich: 0 offen.

**Nachtext-Fix (kopfüber-Text):** Ursache war fehlendes `EarlyFramebuffer`-Port — Java rendert die Layout-Elemente nie direkt auf den Screen, sondern in das 854×480-FBO und blittet es **vertikal gespiegelt** auf das Fenster (`blitToScreen`: „src Y are flipped, since our FB is flipped“); dazu läuft der `screenSize`-Uniform auf der festen Layout-Größe (854×480), nicht auf der Fenstergröße. Portiert: `earlydisplay/EarlyFramebuffer` (1:1 inkl. Aspect-Fit, Theme-Hintergrund #ef323d, geflippter Blit) und der Minecraft-Renderpfad durch die `LoadingScreenRenderer.renderToScreen`-Sequenz. Verifiziert per Frame-Readback (`MATTI_FONT_DUMP=<file>.ppm`): Titel aufrecht an Position 10/20.

────────────────────────────────────────────────────────────────────────────────

## P4 — Welt rendern

| #  | Was                                                                       | Status | Anmerkung |
|----|---------------------------------------------------------------------------|--------|-----------|
| 1  | Chunk-Meshing (ChunkRenderDispatcher, Section-Builder, BlockModel→Mesh)    | ✅     | SectionRenderDispatcher/-Region/-Section, SectionCompiler + VisGraph + VisibilitySet (6-Bit Face-Cull), SectionBufferBuilderPack, SectionBuffers + GpuBuffer-Upload, SectionShader (terrain.vert/frag, ChunkPos-Uniform, VAO-Rebuild bei Buffer-Wechsel), CompiledSectionMesh + ChunkSectionLayer; ChunkMesh-Harness grün |
| 2  | BlockModel/BakedModel (JSON-Modelle, Elemente, Face-Baking, Atlas-Sprites) | ✅     | BlockElement/ElementFace + Gson-Deserializer (cullface 1:1), FaceBakery (defaultFaceUV/bakeVertex/calculateFacing/recalculateWinding), QuadCollection + ModelBaker (SimpleUnbakedGeometry.bake, Cullface-Buckets, Degenerate-Axis-Gate), SpriteGetter (Atlas-UV-Rects), ModelManager (lädt + backt Modelle, Child-Texturen über Parent-Chain), VanillaModels (cube/cube_all) + VanillaBlockModels/VanillaBlockTextures (Texturen in den Block-Atlas gestitcht); AtlasTextures-Harness 24 Checks grün; MVP-Upload column-major verifiziert |
| 3  | Camera + Frustum + Culling                                                 | ✅     | Camera (SetRotation/SetPosition/Move über die Quaternion-Basis, Basisvektoren wie Java: pitch + schaut hoch), FrustumIntersection (Plane-Extraktion 1:1 JOML, m<SPALTE><ZEILE>-Konvention, NZ/PZ = Near/Far), Frustum (calculateFrustum = projection·view wie MCs LevelRenderer.calculateFrustum(view, projection) mit p2.mul(p1), viewVector = transformTranspose((0,0,1,0)), CubeInFrustum/PointInFrustum/IsVisible kamera-relativ), RenderLayer cullt pro Section (AABB im Frustum); Demo-Level: zweite Plattform 4 Sections östlich als Culling-Beweis (yaw-Sweep: Ost-Sicht zeichnet sie, Nord-Sicht cullt sie); frustum-Harness 18 Checks grün; MVP = projection·view (Clip-Raum) |
| 4  | BlockRenderDispatcher (AO, Tinting)                                        | ✅     | IdMapper, GrassColor/FoliageColor-Tabellen, BlockColors.createDefault() mit allen Vanilla-Providern (Gras/Laub/Wasser/Redstone/Stems/Lily), ModelBlockRenderer AO-Kern 1:1 (AdjacencyInfo-Corner-Tables, AmbientVertexRemap, calculateShape, die Open-Checks, Nicht-Cubic-Gewichte, beide blend-Helfer, LightTexture.pack), BlockRenderDispatcher-Facade (AO-Entscheidung, renderBatched), SectionCompiler läuft über den Dispatcher-Pfad, BlockAndTintGetter.getShade; ao-Harness 14 Checks grün |
| 5  | Himmel/Sonne/Mond/Wolken (günstig, sieht sofort nach MC aus)               | ✅     | SkyRenderer (Sky-Disc Day/Night, Sunrise/Sunset-Fan, Sonne/Mond-Phasen, Sterne), CloudRenderer (FANCY, Zeit-Drift) — kamera-relativ über die Rotation-only View (PR #12/#14); World-Pass cleared auf Himmelblau, Blit-Flip über den Y-Flip der Projektion kompensiert; Terrain sampelt den Block-Atlas (Root-Cause des schwarzen Terrains: SimpleFont leckte GL_UNPACK_ROW_LENGTH) |

**Ziel:** du stehst in einer Welt aus echten Blöcken und drehst die Kamera.

**Status: erreicht** — First-Person-Spawn auf der Plattform, Mouse-Look + WASD-Walk über den GLFW-Port (Cursor-Delta dreht, Tasten laufen in der Yaw-Ebene), texturiertes Terrain, Himmel/Sonne/Mond/Sterne/Wolken über dem Spieler.

────────────────────────────────────────────────────────────────────────────────

## P5 — Spieler & Interaktion

| # | Was                                                                 | Status |
|---|---------------------------------------------------------------------|--------|
| 1 | Entity-Hierarchie + EntityType-Registry                             | ✅     |
| 2 | Player/LocalPlayer, Input/KeyMapping (GLFW-Port liefert die Tasten) | ✅     |
| 3 | Physik/Kollision (AABB-Sweep gegen BlockStates)                     | ✅     |
| 4 | Raycast/HitResult (Block klicken)                                   | ✅     |
| 5 | GameRenderer/HUD (Crosshair, Hotbar)                                | ✅     |
| 6 | Sound-Engine (OpenAL-Binding)                                       | ✅     |

**Ziel:** du läufst durch die Welt, setzt/brichst Blöcke — der „es ist Minecraft“-Moment.

────────────────────────────────────────────────────────────────────────────────

## P6 — Inventar/GUI

| # | Was                                                                        | Status |
|---|----------------------------------------------------------------------------|--------|
| 1 | Menu/Slot-System (MenuType, AbstractContainerMenu)                          | ✅     |
| 2 | Screen-Hierarchie (Screen, Button, TextField — passt zum Mixin-Hook-System) | ✅     |
| 3 | Inventory/Hotbar-Daten, ItemRenderer                                        | ⬜     |
| 4 | Pause/Options-Screen (options.txt über ConfigTracker — Config-Arbeit zahlt sich hier aus) | ⬜ |

────────────────────────────────────────────────────────────────────────────────

## P7 — Speichern & Generierung

| # | Was                                                                          | Status |
|---|------------------------------------------------------------------------------|--------|
| 1 | LevelStorage + Region/Anvil-Format (über NBT aus P1)                          | ⬜     |
| 2 | Chunk-Generierung (Noise-Shader, Biome-Quelle — erst simple Superflat)        | ⬜     |
| 3 | DataFixerUpper (vendor/DataFixerUpper liegt vor — erst für alte Welten nötig) | ⬜     |
| 4 | Entities speichern/laden                                                      | ⬜     |

────────────────────────────────────────────────────────────────────────────────

## P8 — Multiplayer (optional, zuletzt)

| # | Was                                                                    | Status |
|---|------------------------------------------------------------------------|--------|
| 1 | Connection/Channel über den vorhandenen Netty-Port, ConnectionProtocol, PacketListener | ⬜ |
| 2 | ClientPacketListener + Login/Play-Pakete                                | ⬜     |
| 3 | Server-Port (eigener großer Ast — vorher klären, ob du überhaupt willst) | ⬜     |

────────────────────────────────────────────────────────────────────────────────

## Parallel-Strang: Mod-API-Vorderseite (damit Mods davon profitieren)

| Was                                                                       | Status | Anmerkung |
|---------------------------------------------------------------------------|--------|-----------|
| DeferredRegister/RegistryObject → hängt an P1                              | 🟨     | RegisterEvent-Pfad steht; DeferredRegister-Bequemlichkeit fehlt noch |
| Block/Item-Events, tick-Events → EventBus vorhanden                        | 🟨     | EventBus hängt am Mod-Ladevorgang; Game-Events (tick) kommen mit P5 |
| Mixin-Targets im Game-Code mit `MATTI_MIXIN_TARGET` markieren (M4-Infrastruktur) | 🟨 | Infrastruktur steht + Example-Mod mixt in `matticraft::demo::*`; Game-Targets kommen mit P2/P4-Klassen |

────────────────────────────────────────────────────────────────────────────────

## Test-Harness-Übersicht (alles unter `tests/`, gebaut über CMake, `ctest`)

| Harness      | Deckt ab                                                                    |
|--------------|-----------------------------------------------------------------------------|
| `nbt`        | SNBT parse/print, binär, gzip, ListTag-Wrapper                              |
| `math`       | Mth, Vec3, BlockPos, ChunkPos, AABB, Direction                              |
| `blockstate` | Property/StateHolder/StateDefinition/BlockState (37 Checks)                 |
| `vanilla`    | Vanilla-Bootstrap: Registry-Counts, Ordnung, Defaults, BlockItem-Backlinks  |
| `itemstack`  | ItemStack + DataComponents (27 Checks)                                      |
| `brigadier`  | Dispatcher parse/execute/usage                                              |
| `modscan`    | `.so`-Mod-Scan + TOML-Metadaten                                             |
| `soscan`     | ELF-Sektionen der Mods (Manifest/Hook-Tabelle)                              |
| `mixinfacade`| M4-Hook-Tabelle + M2-GOT-Patcher Fixtures                                   |
| `m4proof`    | Mixin-Backend-Proof                                                         |
| `level`      | Level/LevelChunk/Section/Heightmap/BlockEntity (36 Checks)                  |
| `gl`         | GL-Binding: Konstanten, dlopen-Verfügbarkeit, Fallbacks ohne Treiber (44 Checks) |
| `window`     | Minecraft-Run-Loop: init, 20-TPS-Ticks, Mixin-Dispatch, Stop/Destroy        |
| `rendersystem` | RenderSystem/GlStateManager State-Cache, VertexArrayCache, GpuBuffer      |
| `bufferbuilder` | ByteBufferBuilder/BufferBuilder/MeshData/VertexFormat/Tesselator (49 Checks) |
| `font`       | SimpleFont-Pack über Monocraft.ttf, Glyph-Metriken (11 Checks)              |
| `shader`     | GlslPreprocessor (moj_import), GlShaderModule/GlProgram, ShaderManager      |
| `texture`    | NativeImage/PNG, Stitcher, SpriteLoader, TextureAtlas-Stitching (11 Checks) |
| `chunkmesh`  | P4.1-Compile-Pipeline über eine echte Level-Region (VisGraph, Layer-Packing) |
| `atlastextures` | P4.2: Vanilla-Modelle backen gegen den echten Block-Atlas (24 Checks)    |
| `ao`         | P4.4: AO-Kern über echtem Level, BlockColors-Provider, Shade-Tabelle, Tesselate-Pfad (14 Checks) |

Run-Configurations (CLion): **runClient** (der eine Client mit allem), **build mods** (baut alle Mods aus `mods/`), **matticraft** (Client mit Tests).

────────────────────────────────────────────────────────────────────────────────

## Nächste sinnvolle Schritte (Reihenfolge-Vorschlag)

1. ~~P4.3 Camera + Frustum + Culling~~ ✅ erledigt — Camera/Frustum/FrustumIntersection portiert, Dispatcher cullt pro Section (Yaw-Sweep im Client verifiziert).
2. ~~P4.4 BlockRenderDispatcher (AO, Tinting)~~ ✅ erledigt — AO-Kern 1:1 (AdjacencyInfo/Remap/calculateShape/blend), BlockColors mit allen Vanilla-Providern, Dispatcher-Facade, SectionCompiler über renderBatched; ao-Harness grün.
3. ~~P4.5 Himmel/Sonne/Mond/Wolken~~ ✅ erledigt — Sky-Disc (Day/Night-Farbe), Sunrise/Sunset-Fan, Sonne/Mond-Phasen über die prozedurale Celestials-Atlas, Sterne, CloudRenderer (FANCY-Zellen, Zeit-Drift); kamera-relatives Rendern über die Rotation-only View wie Java (PR #12/#14).
4. ~~**P0-Rest:** java.util-Lücken~~ ✅ erledigt — UUID (Bits, Version/Variant, Name-UUID via MD5, Random-UUID, kanonischer String-Roundtrip), BitSet (long[]-Worte, Grow, and/or/xor/andNot, cardinality/nextSetBit, toLongArray), Collections.unmodifiableList/Map/Set (Read-Views, Mutatoren fail-fast mit UnsupportedOperationException).
5. ~~**P2-Nachtrag:** PalettedContainer, EntityBlock, BlockEntity-NBT~~ ✅ erledigt — PalettedContainer + SimpleBitStorage (SINGLE_VALUE/LINEAR/GLOBAL-Strategien, Bits-Wachstum, Pack/Unpack) in LevelChunkSection; EntityBlock-VTable am Block (newBlockEntity + Typ-Slot, Valid-Blocks-Scan als Vanilla-Fallback); BlockEntity-NBT (saveMetadata/saveWithFullMetadata/saveWithId/saveCustomOnly/loadStatic über NbtIo).
6. ~~**P5.1:** Entity-Hierarchie + EntityType-Registry~~ ✅ erledigt — EntityAttachments/EntityDimensions/MobCategory, Entity-Basis (Pos/Rot/BBox, Tags, NBT-Roundtrip, Removal-Lifecycle), EntityType-Builder + Registry, 13 Vanilla-Typen, Level-Entity-Surface (Add/Remove/GetEntitiesInBox); entity-Harness grün.
7. ~~**P5.2:** Player/LocalPlayer, Input/KeyMapping~~ ✅ erledigt — Abilities, FoodData/FoodConstants (Eat/Exhaustion/NBT), Player-Struct (Health-Clamp, abilities/food Defaults, Player-NBT mit XpP/Score/abilities-Subtag), KeyMapping-Registry (ALL/MAP-Tabellen, Set/Click/ReleaseAll, Vanilla-Bindings mit Mouse-Typen), Input-Record + Vec2 (MoveVector-Normalize), LocalPlayer (KeyboardInput-Tick 1:1, Java-Strafe-Konvention +1 links); Client fährt Camera/Position über die LocalPlayer-Entity (Mouse-Look über Entity.turn), player-Harness grün.
8. ~~**P5.3:** Physik/Kollision~~ ✅ erledigt — Level-Blockkollisionen (GetBlockCollisions/NoBlockCollision über hasCollision-Gate), Entity-Collide-Sweep (axisStepOrder |x|<|z| ? YZX : YXZ, Shapes.collide-Reduktion per Achse), Entity_Move (MoverType, noPhysics-Pfad, Collision-Flags, On-Ground, Horizontal-Motion-Reset); Client läuft auf Gravity (-0.08·0.98) + Vanilla-Jump (+0.42) über Entity_Move mit Slipperiness-Friction (0.546 am Boden / 0.91 in der Luft) pro Tick, physics-Harness grün; Client-Bugfixes: DeltaTracker-Epochen-Seed, runTick-flag, Vanilla-KeyMappings im Client (P5.3-Bugs behoben, xdotool-verifiziert).
9. ~~**P5.4:** Raycast/HitResult~~ ✅ erledigt — BlockHitResult (Miss/Block/Entity-Typ, Face-Direction, Inside-Pfad wie VoxelShape.clip), ClipContext (Block/Fluid-Modi, Shape-Getter), Level_Clip (BlockGetter.traverseBlocks-DDA 1:1 mit ±1e-7-Lerp, per-Zelle AABB-Clip über die hasCollision-Gate), Client: Crosshair-Pick pro Frame (Reach 4.5, Creative), Linksklick bricht den anvisierten Block, Rechtsklick platziert an der Entry-Face, Section-Re-Mesh um die Änderung; pick-Harness 27 Checks grün, Client-Interaktion per xdotool-Mausklick verifiziert (Break → Loch in der Plattform, Place → neuer Block vom Pick getroffen).
10. **Run-Configs/CI:** Stable-Release (workflow_dispatch: AppImage, Portable-Zip, Dev-SDK, `.matti`-Mods, Changelog, SHA256SUMS) steht — siehe `.github/WORKFLOWS.md` für alle Actions.

// The M4 harness fixture mod. Empty source on purpose: the manifest comes in
// through matti_embed_section (see tests/CMakeLists.txt). The harness reads the
// .so through JarContents (the SoScan path) and hooks the executable's
// "matticraft::demo::tick" target through its .matti_mixins section.
//
// The .matti_mixins hook is appended by the CMake fixture below.

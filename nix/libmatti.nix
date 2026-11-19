{ lib
, stdenv
, cmake
, ninja
, pkg-config
, rustc
, cargo
, zlib
, sdl3
, vulkan-loader
, vulkan-headers
, stbTruetype
, stbImage
, monocraft
, src ? ../.
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "libmatti";
  version = "0.1.0";
  inherit src;
  outputs = [ "out" "dev" ];

  nativeBuildInputs = [ cmake ninja pkg-config rustc cargo ];
  buildInputs = [ zlib sdl3 vulkan-loader vulkan-headers ];

  postUnpack = ''
    install -Dm644 ${stbTruetype} source/vendor/stb/stb_truetype.h
    install -Dm644 ${stbImage} source/vendor/stb/stb_image.h
    install -Dm644 ${monocraft} source/vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf
  '';

  cmakeFlags = [
    "-DMATTI_MOBILE_ONLY=ON"
    "-DMATTI_BUILD_LAUNCHER=OFF"
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  buildPhase = ''
    runHook preBuild
    cmake --build . --target matti_shared -j "$NIX_BUILD_CORES"
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p "$out/lib" "$dev/include/libmatti" "$out/share/doc/matticraft"
    cp library/libmatti.so.0.1.0 "$out/lib/"
    ln -s libmatti.so.0.1.0 "$out/lib/libmatti.so.0"
    ln -s libmatti.so.0 "$out/lib/libmatti.so"

    (cd "${finalAttrs.src}/library/src/libmatti"
     find . -name '*.h' -print0 |
       while IFS= read -r -d "" header; do
         install -Dm644 "$header" "$dev/include/libmatti/''${header#./}"
       done)

    if [ -d "${finalAttrs.src}/library/src/libmatti/matti/ui/assets" ]; then
      mkdir -p "$out/share/matticraft"
      cp -a "${finalAttrs.src}/library/src/libmatti/matti/ui/assets" "$out/share/matticraft/"
    fi
    if [ -f ../vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf ]; then
      install -Dm644 \
        ../vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf \
        "$out/share/matticraft/Monocraft.ttf"
    fi
    install -Dm644 "${finalAttrs.src}/README.md" "${finalAttrs.src}/planported.MD" "$out/share/doc/matticraft/"

    runHook postInstall
  '';

  meta = with lib; {
    description = "Matticraft's C11 game library (shared library and public headers)";
    homepage = "https://github.com/mattiasploesch-cpu/LibMatti";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [ ];
    outputsToInstall = [ "out" ];
  };
})

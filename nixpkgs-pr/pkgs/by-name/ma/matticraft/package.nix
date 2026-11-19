{ lib
, stdenv
, fetchurl
, libmatti
, cmake
, ninja
, pkg-config
, rustc
, cargo
, zlib
, sdl3
, vulkan-loader
, vulkan-headers
, glfw
, libGL
, makeWrapper
}:

let
  # The game embeds the font and texture files into the executable.
  stbTruetype = fetchurl {
    url = "https://raw.githubusercontent.com/nothings/stb/2c980bb59875b0d32144a71867fbdebb2f77cd20/stb_truetype.h";
    hash = "sha256-7NMLBeDdT+o6E8JoEN2eGZLcN5BJSCw5PVoZ5rUJCqs=";
  };
  stbImage = fetchurl {
    url = "https://raw.githubusercontent.com/nothings/stb/2c980bb59875b0d32144a71867fbdebb2f77cd20/stb_image.h";
    hash = "sha256-WUwv411JSItDgtv67I+YNm3vyoGdkWrJW+zz519CALM=";
  };
  monocraft = fetchurl {
    url = "https://raw.githubusercontent.com/neoforged/FancyModLoader/666fa649f1cb8edd7d21665f60423ecd8081620c/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf";
    hash = "sha256-th7jJW9EnpYUD1RRWBlUCEDvDK3oJxHro1mlhkx3kHY=";
  };
in
stdenv.mkDerivation (finalAttrs: {
  pname = "matticraft";
  version = "0.1.0";

  # libmatti carries the pinned source; the client is built from the same tree.
  src = libmatti.src;

  nativeBuildInputs = [ cmake ninja pkg-config rustc cargo makeWrapper ];
  buildInputs = [ zlib sdl3 vulkan-loader vulkan-headers glfw libGL ];

  postUnpack = ''
    install -Dm644 ${stbTruetype} "$sourceRoot/vendor/stb/stb_truetype.h"
    install -Dm644 ${stbImage} "$sourceRoot/vendor/stb/stb_image.h"
    install -Dm644 ${monocraft} "$sourceRoot/vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf"
  '';

  cmakeFlags = [
    "-DMATTI_BUILD_LAUNCHER=OFF"
    "-DMATTI_CLIENTS_SHARED=ON"
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_SKIP_RPATH=ON"
  ];

  buildPhase = ''
    runHook preBuild
    cmake --build . --target matti_shared runClient vulkanClient -j "$NIX_BUILD_CORES"
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p "$out/bin" "$out/lib" "$out/share/matticraft"
    install -Dm755 runClient "$out/bin/matticraft"
    install -Dm755 vulkanClient "$out/bin/vulkanClient"
    cp -a library/libmatti.so* "$out/lib/"
    if [ -d ../library/src/libmatti/matti/ui/assets ]; then
      cp -a ../library/src/libmatti/matti/ui/assets "$out/share/matticraft/"
    fi
    if [ -f ../vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf ]; then
      install -Dm644 \
        ../vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf \
        "$out/share/matticraft/Monocraft.ttf"
    fi
    install -Dm644 ../README.md "$out/share/doc/matticraft/README.md"
    install -Dm644 ../planported.MD "$out/share/doc/matticraft/planported.MD"

    for entry in matticraft vulkanClient; do
      mv "$out/bin/$entry" "$out/bin/.$entry"
      makeWrapper "$out/bin/.$entry" "$out/bin/$entry" \
        --set MATTI_UI_ASSETS "$out/share/matticraft/assets" \
        --set MATTI_THEME_FONT "$out/share/matticraft/Monocraft.ttf" \
        --prefix LD_LIBRARY_PATH : "$out/lib"
    done

    runHook postInstall
  '';

  meta = with lib; {
    description = "Matticraft client and libmatti shared library";
    homepage = "https://github.com/mattiasploesch-cpu/LibMatti";
    license = with licenses; [ mit lgpl3Plus ];
    platforms = platforms.linux;
    maintainers = [ ];
    mainProgram = "matticraft";
  };
})

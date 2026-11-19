{ lib
, stdenv
, fetchFromGitHub
, fetchurl
, cmake
, ninja
, pkg-config
, rustc
, cargo
, zlib
, sdl3
, vulkan-loader
, vulkan-headers
}:

let
  # These files live in git submodules; pin the files used by the build.
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
  pname = "libmatti";
  version = "0.1.0";

  src = fetchFromGitHub {
    owner = "mattiasploesch-cpu";
    repo = "LibMatti";
    rev = "a0459d3a678b3ff2465a85cb54b972cad5e9fad6";
    hash = "sha256-4GcSy4pMpb2mqU/ndnY4IvVkiiMNUFw60NLjEyOKPUM=";
  };

  outputs = [ "out" "dev" ];

  nativeBuildInputs = [ cmake ninja pkg-config rustc cargo ];
  buildInputs = [ zlib sdl3 vulkan-loader vulkan-headers ];

  postUnpack = ''
    install -Dm644 ${stbTruetype} "$sourceRoot/vendor/stb/stb_truetype.h"
    install -Dm644 ${stbImage} "$sourceRoot/vendor/stb/stb_image.h"
    install -Dm644 ${monocraft} "$sourceRoot/vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf"
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

    (cd ../library/src/libmatti
     find . -name '*.h' -print0 |
       while IFS= read -r -d "" header; do
         install -Dm644 "$header" "$dev/include/libmatti/''${header#./}"
       done)

    if [ -d ../library/src/libmatti/matti/ui/assets ]; then
      mkdir -p "$out/share/matticraft"
      cp -a ../library/src/libmatti/matti/ui/assets "$out/share/matticraft/"
    fi
    if [ -f ../vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf ]; then
      install -Dm644 \
        ../vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf \
        "$out/share/matticraft/Monocraft.ttf"
    fi
    install -Dm644 ../README.md "$out/share/doc/matticraft/README.md"
    install -Dm644 ../planported.MD "$out/share/doc/matticraft/planported.MD"

    runHook postInstall
  '';

  meta = with lib; {
    description = "Matticraft's C11 game library (shared library and public headers)";
    homepage = "https://github.com/mattiasploesch-cpu/LibMatti";
    license = with licenses; [ mit lgpl3Plus ];
    platforms = platforms.linux;
    maintainers = [ ];
  };
})

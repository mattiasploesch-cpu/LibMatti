{ lib
, rustPlatform
, pkg-config
, makeWrapper
, libxkbcommon
, wayland
, libGL
, xorg
, libmatti
}:

rustPlatform.buildRustPackage {
  pname = "matticraft-launcher";
  version = "0.1.0";
  src = libmatti.src;

  cargoRoot = "launchers";
  cargoLock.lockFile = "${libmatti.src}/launchers/Cargo.lock";
  cargoBuildFlags = [ "--package" "launcher-desktop" "--bin" "matticraft-launcher" ];

  preBuild = ''
    cd launchers
  '';

  nativeBuildInputs = [ pkg-config makeWrapper ];
  buildInputs = [
    libxkbcommon
    wayland
    libGL
    xorg.libX11
    xorg.libXcursor
    xorg.libXrandr
    xorg.libXi
    xorg.libXext
    xorg.libxcb
  ];

  postInstall = ''
    wrapProgram "$out/bin/matticraft-launcher" \
      --prefix LD_LIBRARY_PATH : "${
        lib.makeLibraryPath [
          libGL
          libxkbcommon
          wayland
          xorg.libX11
          xorg.libXcursor
          xorg.libXrandr
          xorg.libXi
          xorg.libXext
          xorg.libxcb
        ]
      }"
  '';

  meta = with lib; {
    description = "Matticraft desktop launcher (Rust/egui)";
    homepage = "https://github.com/mattiasploesch-cpu/LibMatti";
    license = licenses.lgpl3Plus;
    platforms = platforms.linux;
    maintainers = [ ];
    mainProgram = "matticraft-launcher";
  };
}

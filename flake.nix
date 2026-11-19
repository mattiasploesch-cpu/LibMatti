{
  description = "LibMatti / Matticraft";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = f:
        nixpkgs.lib.genAttrs systems
          (system: f system (import nixpkgs { inherit system; }));
    in {
      packages = forAllSystems (system: pkgs:
        let
          # These files live in git submodules. Fetch only the pinned files so
          # `nix build` does not depend on the checkouts being initialised.
          stbTruetype = pkgs.fetchurl {
            url = "https://raw.githubusercontent.com/nothings/stb/2c980bb59875b0d32144a71867fbdebb2f77cd20/stb_truetype.h";
            hash = "sha256-7NMLBeDdT+o6E8JoEN2eGZLcN5BJSCw5PVoZ5rUJCqs=";
          };
          stbImage = pkgs.fetchurl {
            url = "https://raw.githubusercontent.com/nothings/stb/2c980bb59875b0d32144a71867fbdebb2f77cd20/stb_image.h";
            hash = "sha256-WUwv411JSItDgtv67I+YNm3vyoGdkWrJW+zz519CALM=";
          };
          monocraft = pkgs.fetchurl {
            url = "https://raw.githubusercontent.com/neoforged/FancyModLoader/666fa649f1cb8edd7d21665f60423ecd8081620c/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf";
            hash = "sha256-th7jJW9EnpYUD1RRWBlUCEDvDK3oJxHro1mlhkx3kHY=";
          };
        in {
          libmatti = pkgs.callPackage ./nix/libmatti.nix {
            src = self;
            inherit stbTruetype stbImage monocraft;
          };
          libmatti-dev = pkgs.runCommand "libmatti-0.1.0-dev" { } ''
            mkdir -p "$out"
            cp -a ${self.packages.${system}.libmatti.dev}/. "$out/"
          '';
          matticraft = pkgs.callPackage ./nix/matticraft.nix {
            src = self;
            inherit stbTruetype stbImage monocraft;
          };
          default = self.packages.${system}.matticraft;
        });

      devShells = forAllSystems (system: pkgs: {
        default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            cmake ninja pkg-config gcc rustc cargo
          ];
          buildInputs = with pkgs; [
            zlib sdl3 vulkan-loader vulkan-headers glfw libGL
          ];
        };
      });
    };
}

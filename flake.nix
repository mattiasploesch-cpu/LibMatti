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
      packages = forAllSystems (system: pkgs: {
          libmatti = pkgs.callPackage ./nix/libmatti.nix { src = self; };
          libmatti-dev = pkgs.runCommand "libmatti-0.1.0-dev" { } ''
            mkdir -p "$out"
            cp -a ${self.packages.${system}.libmatti.dev}/. "$out/"
          '';
          matticraft = pkgs.callPackage ./nix/matticraft.nix { src = self; };
          matticraft-launcher = pkgs.callPackage ./nix/launcher.nix { src = self; };
          default = self.packages.${system}.matticraft;
        });

      # NUR/flake users can import this overlay. Nixpkgs deliberately stays
      # downstream of the flake until the project has the required user base.
      overlays.default = final: prev: {
        libmatti = final.callPackage ./nix/libmatti.nix { src = self; };
        matticraft = final.callPackage ./nix/matticraft.nix { src = self; };
        matticraft-launcher = final.callPackage ./nix/launcher.nix { src = self; };
      };

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

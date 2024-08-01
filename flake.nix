{
  description = "bin2x2 test suite";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    let
      systems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];
      targets = [
        "x86_64-linux" "x86_64-darwin" "x86_64-windows"
        "aarch64-linux" "aarch64-darwin" "aarch64-windows"
      ];
    in
    flake-utils.lib.eachSystem systems (system:
      let
        pkgs = import nixpkgs { inherit system; };
        bin2x2 = import ./default.nix { inherit pkgs; };

        crossPkgs = builtins.listToAttrs (map (target: {
          name = target;
          value = import nixpkgs {
            inherit system;
            crossSystem = pkgs.lib.systems.elaborate target;
          };
        }) targets);

        buildAllTests = builtins.mapAttrs (target: crossPkgs':
          bin2x2.buildAllTests { 
            stdenv = crossPkgs'.stdenv; 
            doCheck = false;
          }
        ) crossPkgs;

        buildAllTestsNative = bin2x2.buildAllTests { stdenv = pkgs.stdenv; };

      in {
        packages = {
          default = buildAllTestsNative;
        } // buildAllTests;

        apps = builtins.mapAttrs (name: package: {
          type = "app";
          program = "${package}/bin/${builtins.head (builtins.attrNames (builtins.readDir "${package}/bin"))}";
        }) buildAllTests;

        devShells.default = import ./shell.nix { inherit pkgs; };
      }
    );
}

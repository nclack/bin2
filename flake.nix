{
  description = "bin2 project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        project = import ./default.nix { inherit pkgs; };
      in
      {
        packages.default = project;
        devShell = import ./shell.nix { inherit pkgs; };
      }
    );
}

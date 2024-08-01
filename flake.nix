{
  description = "bin2x2 test suite";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachSystem ["x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin"] (system:
      let
        pkgs = import nixpkgs { inherit system; };
        bin2x2 = import ./default.nix { inherit pkgs; };
        allTests = builtins.concatMap
          (impl: builtins.concatMap
            (target: builtins.map
              (os: bin2x2.buildTest ({ inherit (impl) name src; inherit target os; }))
              bin2x2.oses)
            bin2x2.targets)
          bin2x2.impls;
      in {
        packages = {
          default = bin2x2.buildAllTests;
        } // builtins.listToAttrs (map (test: { name = test.name; value = test; }) allTests);

        apps = builtins.listToAttrs (map (test: {
          name = test.name;
          value = {
            type = "app";
            program = "${test}/bin/${builtins.baseNameOf test.name}";
          };
        }) allTests);
      }
    );
}

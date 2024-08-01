{ pkgs ? import <nixpkgs> {} }:

let
  llvmPackages = pkgs.llvmPackages_latest;

  buildTest = { name, src, target, os }:
    let
      flags = if builtins.match ".*avx2.*" name != null then [ "-mavx2" ]
              else if builtins.match ".*neon.*" name != null then [ "-mfpu=neon" ]
              else [];
      targetFlags = if target == "x86_64" then [ "-target x86_64-${os}" ]
                    else if target == "aarch64" then [ "-target aarch64-${os}" ]
                    else [];
      osFlags = if os == "windows" then [ "-D_WIN32" ]
                else if os == "darwin" then [ "-D__APPLE__" ]
                else [];
    in pkgs.stdenv.mkDerivation {
      pname = "bin2x2-test-${name}-${target}-${os}";
      version = "0.1.0";
      src = ./.;
      nativeBuildInputs = [ llvmPackages.clang ];
      buildPhase = ''
        ${llvmPackages.clang}/bin/clang -O3 ${pkgs.lib.escapeShellArgs (flags ++ targetFlags ++ osFlags)} -o bin2x2_test_${name}_${target}_${os} test/test.c src/${src}
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp bin2x2_test_${name}_${target}_${os} $out/bin/
      '';
    };

  targets = [ "x86_64" "aarch64" ];
  oses = [ "linux" "darwin" "windows" ];
  impls = [
    { name = "avx2_naive"; src = "bin2.avx2.naive.c"; }
    { name = "avx2_v2"; src = "bin2.avx2.v2.c"; }
    { name = "neon_naive"; src = "bin2.neon.naive.c"; }
    { name = "neon_v2"; src = "bin2.neon.v2.c"; }
  ];

in {
  inherit buildTest targets oses impls;

  buildAllTests = pkgs.stdenv.mkDerivation {
    name = "bin2x2-tests";
    buildInputs = builtins.concatMap
      (impl: builtins.concatMap
        (target: builtins.map
          (os: buildTest ({ inherit (impl) name src; inherit target os; }))
          oses)
        targets)
      impls;
    phases = [ "installPhase" ];
    installPhase = ''
      mkdir -p $out/bin
      for input in $buildInputs; do
        cp $input/bin/* $out/bin/
      done
    '';
  };
}

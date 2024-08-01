{ pkgs ? import <nixpkgs> {} }:

let
  buildTest = { stdenv, name, src, target, os }:
    let
      isX86 = stdenv.hostPlatform.isx86;
      isArm = stdenv.hostPlatform.isAarch64;
      
      extraFlags = if isX86 && builtins.match ".*avx2.*" name != null then "-mavx2"
                   else if isArm then "-march=armv8-a+simd" # This is more universally supported for AArch64
                   else "";
    in
    stdenv.mkDerivation {
      pname = "bin2x2-test-${name}";
      version = "0.1.0";
      src = ./.;

      buildPhase = ''
        $CC -O3 ${extraFlags} -o bin2x2_test_${name} test/test.c src/${src}
      '';

      checkPhase = ''
        ./bin2x2_test_${name}
      '';

      installPhase = ''
        mkdir -p $out/bin
        cp bin2x2_test_${name} $out/bin/
      '';
    };

  impls = [
    { name = "plain"; src = "bin2.plain.c"; }
    { name = "avx2_naive"; src = "bin2.avx2.naive.c"; }
    { name = "avx2_v2"; src = "bin2.avx2.v2.c"; }
    { name = "neon_naive"; src = "bin2.neon.naive.c"; }
    { name = "neon_v2"; src = "bin2.neon.v2.c"; }
  ];

in {
  inherit buildTest impls;

  buildAllTests = { stdenv }:
    let
      relevantImpls = (if stdenv.hostPlatform.isx86 
                      then builtins.filter (impl: builtins.match ".*avx2.*" impl.name != null) impls
                      else if stdenv.hostPlatform.isAarch64
                      then builtins.filter (impl: builtins.match ".*neon.*" impl.name != null) impls
                      else []) ++
                      (builtins.filter (impl: impl.name == "plain") impls);
    in
    stdenv.mkDerivation {
      name = "bin2x2-tests";
      buildInputs = map (impl: buildTest { 
        inherit stdenv; 
        inherit (impl) name src; 
        target = stdenv.hostPlatform.config; 
        os = stdenv.hostPlatform.config; 
      }) relevantImpls;
      phases = [ "installPhase" ];
      installPhase = ''
        mkdir -p $out/bin
        for input in $buildInputs; do
          cp $input/bin/* $out/bin/
        done
      '';
    };
}

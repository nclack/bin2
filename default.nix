{ pkgs ? import <nixpkgs> {} }:

let
  inherit (pkgs) lib stdenv clangStdenv;

  isAarch64 = stdenv.hostPlatform.isAarch64;
  isX86_64 = stdenv.hostPlatform.isx86_64;

in clangStdenv.mkDerivation {
  pname = "bin2";
  version = "0.1.0";
  src = ./.;

  buildPhase = ''
    # Build plain test (always)
    $CC -O3 -o test-plain test/test.c src/bin2.plain.c -lm

    # Build AVX2 tests (x86_64 only)
    ${lib.optionalString isX86_64 ''
      $CC -O3 -o test-avx2-naive test/test.c src/bin2.avx2.naive.c -lm
      $CC -O3 -o test-avx2-v2 test/test.c src/bin2.avx2.v2.c -lm
    ''}

    # Build NEON tests and benchmark (AArch64 only)
    ${lib.optionalString isAarch64 ''
      $CC -O3 -o test-neon-naive test/test.c src/bin2.neon.naive.c -lm
      $CC -O3 -o test-neon-v2 test/test.c src/bin2.neon.v2.c -lm
      $CC -O3 -o benchmark-neon-in-vs-out-of-place bench/cumsum-in-vs-out-of-place-neon.c -lm
    ''}
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp test-* $out/bin/
    ${lib.optionalString isAarch64 "cp benchmark-neon-in-vs-out-of-place $out/bin/"}
  '';

  meta = with lib; {
    description = "bin2 project with architecture-specific optimizations";
    platforms = platforms.all;
  };
}



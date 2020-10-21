# TODO why isn't the new version being used in sonicparanoid?
# TODO is an old version actually required by sonicparanoid?
# TODO is there a way to avoid the myEnv thing?
# TODO does llvmPackages.openmp have to match gcc8Stdenv on linux?

{ stdenv
, bzip2
, cmake
, fetchurl
, llvmPackages
, unzip
, xxd
, zlib
}:

let
  myEnv = if pkgs.stdenv.isDarwin
            then pkgs.llvmPackages.stdenv
            else pkgs.gcc8Stdenv;
in

myEnv.mkDerivation rec {
  name = "mmseqs2-${version}";
  # version = "1-c7a89"; # old version for sonicparanoid
  version = "12-113e3";
  src = ./.;
  buildInputs = [
    cmake
    xxd
    zlib
    bzip2.dev
    unzip
    llvmPackages.openmp
  ];
}

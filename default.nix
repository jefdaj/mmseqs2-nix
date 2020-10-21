# TODO replace perl with xxd?
# TODO why isn't the new version being used in sonicparanoid?
# TODO include a custom check for AVX2 to re-enable it when a user's machine is capable
# TODO is an old version actually required by sonicparanoid?
# TODO why is gcc8Stdenv required? maybe it has to match the llvmPackages_8.openmp version?

{ llvmPackages
, bzip2
, cmake
, fetchurl
# , llvmPackages_8
, unzip
, xxd
, zlib
}:

# let
#   sources = import ./nix/sources.nix {};
#   pkgs = import sources.nixpkgs {};
# in

llvmPackages.stdenv.mkDerivation rec {
  name = "mmseqs2-${version}";

  # version = "1-c7a89"; # version for sonicparanoid
  version = "12-113e3";

  src = ./.;
  # src = fetchurl {
  #   url = "https://github.com/soedinglab/MMseqs2/archive/${version}.zip";
  #   sha256 = "0fm3k2y4qc3830pvgixjjsra8ssfdsfr7yyzrcca42pmr750rwzc";

    # latest official release
    # url = "https://github.com/soedinglab/MMseqs2/archive/${version}.tar.gz";
    # sha256 = "11mm1m9dm7jwdsap4p7p6hl3ghqvxr2k0hjf1mp7jm1g988v79s1";

    # version required by sonicparanoid
    # url = "https://bitbucket.org/salvocos/sonicparanoid/raw/55537a6e37a7767c125db18e2bb22059095c47cb/sonicparanoid/mmseqs2_src/mmseqs.tar.gz";
    # sha256 = "0hrcky0jc2pn9gh8xsxknkr08fkm1xbmqwhhxq8rdvaygdjw4spw";
  # };

  buildInputs = [
    cmake
    xxd
    zlib
    bzip2.dev
    unzip
    llvmPackages.openmp
  ];

  #for when the src is a tarball:
  #   unpackPhase = ''
  #     # bump
  #     mkdir -p $TMPDIR
  #     cd $TMPDIR
  #     mkdir ${name}
  #     cd ${name}
  #     tar xvzf ${src}
  #   '';

  # TODO remove?
  # postUnpack = ''
  #   patchShebangs MMseqs2-${version}
  # '';

  # preConfigure = ''
  #   cmakeFlags="$cmakeFlags -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=$out"
  # '';
}

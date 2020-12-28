# TODO why isn't the new version being used in sonicparanoid?

let
  sources = import ./nix/sources.nix {};
  pkgs    = import sources.nixpkgs {};
  mmseqs2 = pkgs.callPackage ./default.nix {};
in
  mmseqs2

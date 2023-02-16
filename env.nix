let
  nixpkgs = import (builtins.fetchTarball {
    name = "4d2b37a84fad1091b9de401eb450aae66f1a741e.tar.gz";
    url = "https://github.com/nixos/nixpkgs/archive/4d2b37a84fad1091b9de401eb450aae66f1a741e.tar.gz";
    sha256 = "11w3wn2yjhaa5pv20gbfbirvjq6i3m7pqrq2msf0g7cv44vijwgw";
  }) {
    config.allowUnfree = true;
  };
in
with nixpkgs;
with lib;
mkShell {
  name = "my-shell";
  buildInputs = [
    python3
    gcc
    gdb
    llvm
    clang
    cmake
    gbenchmark
    linuxKernel.packages.linux_zen.perf
    python3Packages.scipy
  ];
  shellHook = ''
    export PS1='[nix-shell:\w] '
  '';
}


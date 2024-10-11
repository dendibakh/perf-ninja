{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in {
        devShells.default = with pkgs;
          mkShell {
            packages = [
              (python3.withPackages (ps: with ps; [ scipy ]))
              ninja
              cmake
              clang
            ];

            shellHook = ''
              export CXX=clang++
              export CC=clang
              export CFLAGS="-march=native -mtune=native"
              export CXXFLAGS="-march=native -mtune=native"
            '';
          };

        formatter = pkgs.nixfmt;
      });

}

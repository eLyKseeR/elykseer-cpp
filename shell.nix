{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/9ede5d5b569bb9c5c0e7ed4d89cbc926f9f905d4.tar.gz") {} }:

pkgs.mkShell {

    name = "env";

    #src = ./.;

    # Customizable development requirements
    nativeBuildInputs = [
        pkgs.cmake
        pkgs.ccache
        #pkgs.clang
        pkgs.gnused
        pkgs.git
        pkgs.pandoc
        pkgs.cppcheck
        pkgs.html2text
        pkgs.pkg-config
        pkgs.autoconf pkgs.automake pkgs.libtool pkgs.m4
        pkgs.libassuan pkgs.libgpgerror
        pkgs.gnupg
        pkgs.perl534Packages.DigestSHA3
    ];

    buildInputs = [
        pkgs.openssl
        pkgs.zlib
        pkgs.boost
    ];

    shellHook = ''
      echo 'eLyKseeR nixified environment'
      if [ `uname -s` = "Darwin" ]; then
        export SED=gsed
      else
        export SED=sed
      fi
    '';
}


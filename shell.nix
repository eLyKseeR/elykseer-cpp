with import <nixpkgs> {};

stdenv.mkDerivation rec {
    name = "env";

    #src = ./.;

    # Customizable development requirements
    nativeBuildInputs = [
        cmake
        git
        gnused
        pandoc
        html2text
        swig
        #gcc
        clang
        opam m4
        #global
        cppcheck
        #busybox
        perl538Packages.DigestSHA3
        pkg-config
        autoconf automake libtool
        libassuan libgpg-error
        gnupg
    ];

    buildInputs = [
        openssl
        zlib
        boost
    ];

    shellHook = ''
      echo 'eLyKseeR nixified environment'
      if [ `uname -s` = "Darwin" ]; then
        export SED=gsed
      else
        export SED=sed
      fi
      export SED=sed
      export CC=clang
      export CXX=clang++
    '';

}
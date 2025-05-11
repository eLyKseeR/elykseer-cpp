with import <nixpkgs> {};

stdenv.mkDerivation rec {
    name = "env";

    #src = ./.;

    # Customizable development requirements
    nativeBuildInputs = [
        cmake
        ninja
        ccache
        git
        gnused
        gettext
        texinfo
        swig
        gcc
        llvmPackages_19.clang-tools
        llvmPackages_19.openmp
        lldb_19
        clang_19
        opam m4
        cppcheck
        perl
        perl538Packages.DigestSHA3
        pkg-config
        autoconf automake libtool
        libassuan libgpg-error
        gnupg
    ];

    buildInputs = [
        openssl
        zlib
        zstd
        boost186
    ];

    shellHook = ''
      echo 'eLyKseeR nixified environment'
      if [ `uname -s` = "Darwin" ]; then
        export SED=gsed
      else
        export SED=sed
      fi
      export CC=clang
      export CXX=clang++
    '';

}
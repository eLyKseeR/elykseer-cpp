with import <nixpkgs> {};

stdenv.mkDerivation rec {
    name = "env";

    #src = ./.;

    # Customizable development requirements
    nativeBuildInputs = [
        cmake
        ccache
        clang
        gnused
        gcc
        gdb
        git
        pandoc
        cppcheck
        html2text
        pkg-config
        autoconf automake libtool m4
        libassuan libgpgerror
        gnupg
        perl534Packages.DigestSHA3
    ];

    buildInputs = [
        openssl
        zlib
        boost
    ];

    shellHook = ''
      echo 'eLyKseeR nixified environment'
      export SED=sed
    '';
}


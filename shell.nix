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


with import <nixpkgs> {};

stdenv.mkDerivation rec {
    name = "env";

    #src = ./.;

    # Customizable development requirements
    nativeBuildInputs = [
        cmake
        git
        pandoc
        html2text
    ];

    buildInputs = [
        openssl
        zlib
        boost
    ];

}


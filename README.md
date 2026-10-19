# eLyKseeR

![eLyKseeR](https://www.elykseer.com/wp-content/uploads/2017/11/lxricon2.png)

cryptographic data archive; written in C++; an elixir to stay forever

read [more](https://github.com/eLyKseeR/elykseer-cpp)

[![Compilation of project with submodules and verification by unit tests](https://github.com/eLyKseeR/elykseer-cpp/actions/workflows/CI.yaml/badge.svg?branch=main)](https://github.com/eLyKseeR/elykseer-cpp/actions/workflows/CI.yaml)

# archive format

Since version 5.0.0 the archive format follows the formal specification in
[elykseer-ml](https://github.com/eLyKseeR/elykseer-ml) 0.10.1
([doc/08_Format.md](https://github.com/eLyKseeR/elykseer-ml/blob/main/doc/08_Format.md)):
assemblies are encrypted with AES-256-GCM (the aid is authenticated), chunk identifiers,
data layout and file hashes are the same, and the meta data is written as XML following
the schemas `schema/keys.xsd` (`LXRbackup-*-dbks.xml`) and `schema/fileinformation.xsd`
(`LXRbackup-*-files.xml`). The chunks written for an assembly are byte-identical to those of
elykseer-ml (`tests/cpp/utConformance.cpp`, vectors generated with `tests/conformance/generate.sh`),
and archives written by elykseer-ml are restored by `LXRrestore` from its XML export
(`tests/conformance/check-elykseer-ml.sh`, runs both checks in the elykseer-ml docker image).

Archives written by elykseer-cpp 4.x cannot be read by 5.x; restore them with a 4.x release.

`LXRrestore` and `LXRverify` check the checksum of every block and exit with status 1
if a file cannot be restored completely; partially restored files are removed.

# building

1. code extraction and compilation (in directory `build`)

> $ mkdir -vp build; cd build

> $ cmake -GNinja --install-prefix=$(pwd)/../dist -DCMAKE_BUILD_TYPE=Debug ..

> $ ninja

2. testing (in directory `build`)

> $ ./tests/cpp/utelykseer-cpp


# cross compilation

- build Docker image 'xcompile'
   - `docker build -t xcompile .` 

- launch Docker image 'xcompile'
   - ``docker run -it --rm --user=$(id -u):$(id -g) -v .:/work xcompile``
```sh
export PATH=${HOME}/.local/bin:${PATH}
cd build
cmake --fresh -DCMAKE_TOOLCHAIN_FILE=../support/Toolchain_Darwin_to_Windows.cmake --install-prefix=$(pwd)/../dist -DCMAKE_BUILD_TYPE=Release -GNinja ..
ninja && ninja install/strip
```

# contributing

Contributions are welcome! Before submitting your first pull request, please review our
[Contributor License Agreement](CLA.md) (CLA). The CLA allows us to maintain _eLyKseeR_ under a
dual-licensing model (GPLv3 for open source use, with commercial licenses available for proprietary
applications). It is the same for all eLyKseeR repositories. See [CONTRIBUTING.md](CONTRIBUTING.md) for how to sign.

# copyright

Copyright 2018-2025 by Alexander Diemand

# license

This work is licensed under the 
[GNU General Public License v3](https://www.gnu.org/licenses/gpl.html)


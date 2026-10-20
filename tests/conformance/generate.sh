#!/usr/bin/env bash
# Regenerates tests/cpp/conformance_vectors.hpp with elykseer-ml (the reference).
#
#   tests/conformance/generate.sh <path to an elykseer-ml checkout> [output]
#   tests/conformance/generate.sh --docker <elykseer-ml image> [output]
#
# The generator is built in a temporary directory (lxr_cpp_conformance_tmp)
# inside the elykseer-ml dune workspace, which is removed afterwards.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
usage="usage: $0 <path to elykseer-ml> | --docker <image>  [output]"

if [ "${1:-}" = "--docker" ]; then
    IMAGE=${2:?${usage}}
    OUT=${3:-${HERE}/../cpp/conformance_vectors.hpp}
    # the sources are passed in with tar, the header comes back on stdout
    tar --no-xattrs -C "${HERE}" -cf - gen_vectors.ml dune |
        docker run --rm -i "${IMAGE}" bash -c '
            set -e; eval $(opam env)
            mkdir lxr_cpp_conformance_tmp; tar -C lxr_cpp_conformance_tmp -xf -
            dune build ./lxr_cpp_conformance_tmp/gen_vectors.exe >&2
            cd lxr_cpp_conformance_tmp && ../_build/default/lxr_cpp_conformance_tmp/gen_vectors.exe' > "${OUT}.tmp"
    mv "${OUT}.tmp" "${OUT}"
    echo "wrote ${OUT} (${IMAGE})"
    exit 0
fi

ML=$(cd "${1:?${usage}}" && pwd)
OUT=${2:-${HERE}/../cpp/conformance_vectors.hpp}

TMPD="${ML}/lxr_cpp_conformance_tmp"
trap 'rm -rf "${TMPD}" "${ML}/_build/default/lxr_cpp_conformance_tmp"' EXIT
mkdir -p "${TMPD}"
cp "${HERE}/gen_vectors.ml" "${HERE}/dune" "${TMPD}/"

(cd "${ML}" && dune build ./lxr_cpp_conformance_tmp/gen_vectors.exe)
(cd "${TMPD}" && "${ML}/_build/default/lxr_cpp_conformance_tmp/gen_vectors.exe") > "${OUT}.tmp"
mv "${OUT}.tmp" "${OUT}"
echo "wrote ${OUT} (elykseer-ml $(git -C "${ML}" rev-parse --short HEAD))"

#!/usr/bin/env bash
# Builds the CI image from ci/Dockerfile.ci.
#
#   ci/build-ci-image.sh [version]    default: 1.0.0 -> code.sbclab.net/elykseer/elykseer-cpp-ci:1.0.0
#   ARCHS="amd64" ci/build-ci-image.sh   only build these architectures (default: amd64 arm64)
#   PUSH=0 ci/build-ci-image.sh       only build and load locally, do not push
#
# Builds one image per architecture (<tag>-amd64, <tag>-arm64) with Docker
# (non-OCI) media types and no attestations, pushes them, then combines them
# into a manifest list under <tag>.
#
# The external dependencies are built at the submodule commits recorded in
# HEAD, so commit submodule bumps (and changes to ext/Makefile,
# ext/rapidcheck-cstdint.patch or shell.nix) before building. After such a
# change, build and push a new image, then update the image tag in
# .forgejo/workflows/CI-nix.yaml.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

TAGBASE="code.sbclab.net/elykseer/elykseer-cpp-ci"
VTAG=${1:-1.0.0}
ARCHS=${ARCHS:-amd64 arm64}
PUSH=${PUSH:-1}

TAG="${TAGBASE}:${VTAG}"

# EXT_HASH is computed from HEAD but the image is built from the working
# tree: refuse if they differ, the image would never match in CI
HASHED="shell.nix .gitmodules ext/Makefile ext/rapidcheck-cstdint.patch"
if ! git diff --quiet HEAD -- ${HASHED}; then
    echo "uncommitted changes in: $(git diff --name-only HEAD -- ${HASHED} | tr '\n' ' ')"
    echo "commit them first, the CI image is matched against HEAD"
    exit 1
fi

# the prebuilt deps only cover what ext/Makefile builds;
# the header-only submodules are checked out by CI itself
PREBUILT="ext/os-metrics ext/rapidcheck ext/elykseer-crypto"
EXT_REVS=$(git ls-tree HEAD ext/ | awk -v keep="${PREBUILT}" '
    BEGIN { n = split(keep, k, " "); for (i = 1; i <= n; i++) want[k[i]] = 1 }
    $2 == "commit" && ($4 in want) { printf "%s=%s ", $4, $3 }')
EXT_HASH=$(ci/ext-hash.sh)

echo "building ${TAG} for ${ARCHS}"
echo "  EXT_REVS=${EXT_REVS}"
echo "  EXT_HASH=${EXT_HASH}"

ARCH_TAGS=()
for arch in ${ARCHS}; do
    arch_tag="${TAG}-${arch}"
    ARCH_TAGS+=("${arch_tag}")
    echo "building ${arch_tag}"
    if [ "${PUSH}" = 1 ]; then
        output="type=image,name=${arch_tag},push=true,oci-mediatypes=false"
    else
        output="type=docker,name=${arch_tag}"
    fi
    docker buildx build \
        --platform "linux/${arch}" \
        -f ci/Dockerfile.ci \
        --build-arg "EXT_REVS=${EXT_REVS}" \
        --build-arg "EXT_HASH=${EXT_HASH}" \
        --provenance=false --sbom=false \
        --output "${output}" \
        .
done

if [ "${PUSH}" = 1 ]; then
    echo "combining ${ARCH_TAGS[*]} into ${TAG}"
    docker manifest rm "${TAG}" 2>/dev/null || true
    docker manifest create "${TAG}" "${ARCH_TAGS[@]}"
    docker manifest push "${TAG}"
fi

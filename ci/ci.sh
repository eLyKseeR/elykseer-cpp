#!/usr/bin/env bash
# CI steps, shared by .forgejo/workflows/CI-nix.yaml and .github/workflows/CI.yaml.
# Run inside nix-shell (shell.nix provides the toolchain).
#
#   ci/ci.sh deps     build the external dependencies (ext/Makefile)
#   ci/ci.sh build    Debug build in build/
#   ci/ci.sh test     unit tests and CLI smoke test from build/
#   ci/ci.sh asan     Debug build with ASan/UBSan in build-asan/, then all tests
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

BUILD_TYPE=${BUILD_TYPE:-Debug}

case "${1:-}" in
    deps)
        make -C ext
        ;;
    build)
        cmake --fresh -G Ninja -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -S . -B build
        ninja -C build
        ;;
    test)
        (cd build && ./tests/cpp/utelykseer-cpp --report_level=detailed)
        tests/smoke/cli-roundtrip.sh build
        ;;
    asan)
        cmake --fresh -G Ninja -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DENABLE_SANITIZERS=ON -S . -B build-asan
        ninja -C build-asan
        export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:abort_on_error=1}
        # symbolized stacks: needed for the suppressions in ci/lsan.supp
        if command -v llvm-symbolizer >/dev/null; then
            export ASAN_SYMBOLIZER_PATH=${ASAN_SYMBOLIZER_PATH:-$(command -v llvm-symbolizer)}
        fi
        export LSAN_OPTIONS=${LSAN_OPTIONS:-suppressions=${PWD}/ci/lsan.supp:print_suppressions=0}
        export UBSAN_OPTIONS=${UBSAN_OPTIONS:-print_stacktrace=1:halt_on_error=1}
        (cd build-asan && ctest --output-on-failure)
        ;;
    *)
        echo "usage: $0 deps|build|test|asan" >&2
        exit 2
        ;;
esac

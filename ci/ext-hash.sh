#!/usr/bin/env bash
# Prints a hash identifying the prebuilt external dependencies (ext/).
# It covers the pinned submodule commits, ext/Makefile, the rapidcheck patch
# and shell.nix as committed in HEAD. The CI image stores this hash next to
# its prebuilt ext/ directory; CI only reuses that directory if the hashes
# match. (elykseer-crypto pins its own submodules, so its commit covers them.)
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

sha256() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum; else shasum -a 256; fi
}

# runs outside of nix-shell in CI: stick to bash builtins, git and sha256
{
    git ls-tree HEAD ext/ | while read -r mode type rev path; do
        if [ "$type" = commit ]; then echo "$rev $path"; fi
    done
    git rev-parse HEAD:ext/Makefile HEAD:ext/rapidcheck-cstdint.patch HEAD:shell.nix
} | sha256 | { read -r hash _; echo "$hash"; }

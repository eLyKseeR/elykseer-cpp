#!/usr/bin/env bash
# CLI smoke test: LXRbackup -> LXRverify -> LXRrestore -> cmp
#
#   tests/smoke/cli-roundtrip.sh [build dir]    default: build
#
# Uses a throw-away GnuPG home with a fresh key without passphrase,
# so it does not touch the user's keyring.
set -euo pipefail

BUILD=$(cd "${1:-build}" && pwd)
CLI="${BUILD}/src/cpp/cli"
for b in LXRbackup LXRrestore LXRverify; do
    test -x "${CLI}/${b}" || { echo "missing ${CLI}/${b}"; exit 1; }
done

WORK=$(mktemp -d)
cleanup() {
    gpgconf --homedir "${WORK}/gnupg" --kill all 2>/dev/null || true
    rm -rf "${WORK}"
}
trap cleanup EXIT

export GNUPGHOME="${WORK}/gnupg"
mkdir -m 700 "${GNUPGHOME}"
gpg --batch --quiet --passphrase '' \
    --quick-gen-key "LXR smoke test <smoke@elykseer.test>" default default never
OKEY=$(gpg --batch --with-colons --list-keys smoke@elykseer.test 2>/dev/null | awk -F: '$1 == "fpr" { print $10; exit }')

MYID=424242
mkdir -p "${WORK}/data/sub" "${WORK}/chunks" "${WORK}/meta" "${WORK}/restore"
# a small file and one spanning several blocks
# (empty files are not restored yet, so they are left out)
echo "hello eLyKseeR" > "${WORK}/data/hello.txt"
head -c 3000000 /dev/urandom > "${WORK}/data/sub/random.bin"

# the tools are chatty: keep their output in a log, show its end on failure
# (SMOKE_LOG_LINES=all shows all of it, e.g. for sanitizer reports)
LOG="${WORK}/smoke.log"
run() {
    if ! "$@" >>"${LOG}" 2>&1; then
        echo "failed: $*"
        if [ "${SMOKE_LOG_LINES:-80}" = all ]; then cat "${LOG}"; else tail -n "${SMOKE_LOG_LINES:-80}" "${LOG}"; fi
        exit 1
    fi
}

expect_fail() {
    if "$@" >/dev/null 2>&1; then echo "expected failure: $*"; exit 1; fi
}

cd "${WORK}"

echo "== exit codes on bad arguments"
expect_fail "${CLI}/LXRbackup"
expect_fail "${CLI}/LXRrestore"
expect_fail "${CLI}/LXRverify"

echo "== backup"
run "${CLI}/LXRbackup" -y ${MYID} -n 16 -k "${OKEY}" \
    -x "${WORK}/chunks" -o "${WORK}/meta" -D data

DBKS=$(ls "${WORK}"/meta/LXRbackup-*-dbks.xml)
DBFB=$(ls "${WORK}"/meta/LXRbackup-*-files.xml)
test -n "$(ls -A "${WORK}/chunks")" || { echo "no chunks written"; exit 1; }
# a chunk of this backup, modified below
CHUNK=$(find "${WORK}/chunks" -name '*.lxr' | head -n 1)

FILES=()
while IFS= read -r -d '' f; do FILES+=("$f"); done < <(find data -type f -print0)

echo "== verify"
VARGS=()
for f in "${FILES[@]}"; do VARGS+=(-f "$f"); done
run "${CLI}/LXRverify" -y ${MYID} -x "${WORK}/chunks" -k "${DBKS}" -b "${DBFB}" "${VARGS[@]}"

echo "== restore"
run "${CLI}/LXRrestore" -y ${MYID} -n 16 -x "${WORK}/chunks" -o "${WORK}/restore" \
    -k "${DBKS}" -b "${DBFB}" "${VARGS[@]}"

echo "== compare"
for f in "${FILES[@]}"; do
    r="${WORK}/restore/${f}"
    test -f "$r" || { echo "not restored: $f"; find "${WORK}/restore"; exit 1; }
    cmp "$f" "$r"
done

echo "== restore refuses to overwrite, and fails on unknown files"
expect_fail "${CLI}/LXRrestore" -y ${MYID} -n 16 -x "${WORK}/chunks" -o "${WORK}/restore" \
    -k "${DBKS}" -b "${DBFB}" -f data/hello.txt
mkdir -p "${WORK}/restore2"
expect_fail "${CLI}/LXRrestore" -y ${MYID} -n 16 -x "${WORK}/chunks" -o "${WORK}/restore2" \
    -k "${DBKS}" -b "${DBFB}" -f data/does-not-exist
expect_fail "${CLI}/LXRrestore" -y 434343 -n 16 -x "${WORK}/chunks" -o "${WORK}/restore2" \
    -k "${DBKS}" -b "${DBFB}" -f data/hello.txt

echo "== absolute file names are restored below the output directory"
mkdir -p "${WORK}/abs" "${WORK}/meta2" "${WORK}/restore3"
echo "absolute path" > "${WORK}/abs/file.txt"
run "${CLI}/LXRbackup" -y ${MYID} -n 16 -k "${OKEY}" \
    -x "${WORK}/chunks" -o "${WORK}/meta2" -f "${WORK}/abs/file.txt"
run "${CLI}/LXRrestore" -y ${MYID} -n 16 -x "${WORK}/chunks" -o "${WORK}/restore3" \
    -k "$(ls "${WORK}"/meta2/LXRbackup-*-dbks.xml)" -b "$(ls "${WORK}"/meta2/LXRbackup-*-files.xml)" -f "${WORK}/abs/file.txt"
cmp "${WORK}/abs/file.txt" "${WORK}/restore3/${WORK#/}/abs/file.txt"

echo "== a modified chunk is detected"
printf 'X' | dd of="${CHUNK}" bs=1 seek=4242 conv=notrunc 2>/dev/null
mkdir -p "${WORK}/restore4"
expect_fail "${CLI}/LXRverify" -y ${MYID} -x "${WORK}/chunks" -k "${DBKS}" -b "${DBFB}" "${VARGS[@]}"
if "${CLI}/LXRrestore" -y ${MYID} -n 16 -x "${WORK}/chunks" -o "${WORK}/restore4" \
       -k "${DBKS}" -b "${DBFB}" "${VARGS[@]}" >>"${LOG}" 2>&1; then
    echo "expected failure: restore with a modified chunk"; exit 1
fi
# no partially restored files are left
test -z "$(find "${WORK}/restore4" -type f)" || { echo "partial files left:"; find "${WORK}/restore4" -type f; exit 1; }

echo "smoke test passed (${#FILES[@]} files)"

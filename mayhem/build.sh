#!/usr/bin/env bash
#
# mayhem/build.sh — builds the sanitized fuzz harnesses (+ their -standalone reproducers) and the
# Catch2 functional suite for the FiveM/FXServer network-parsing code. Runs as `mayhem` in /mayhem.
#
# Only the self-contained, platform-neutral parts of the tree are built (net-base, net-packet,
# shared/state) with a tiny StdInc.h stand-in (mayhem/include) — the full game/server needs the
# Windows/premake toolchain and ~150 submodules, none of which are required here and none are fetched.
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

SRC="${SRC:-/mayhem}"
cd "$SRC"

OUT="$SRC/out"   # upstream .gitignore already covers /out/; kept outside mayhem/ so no binary lands under it
CODE="$SRC/code"
INC=(-I"$SRC/mayhem/include" -I"$SRC/mayhem" -I"$CODE/shared" -I"$CODE/components/net-base/include" -I"$CODE/components/net-packet/include")
CXXSTD=(-std=c++17 -Wno-parentheses)

rm -rf "$OUT"
mkdir -p "$OUT/fuzz" "$OUT/test"

# 1) Test suite first, with the project's NORMAL flags (no sanitizers) — test.sh only runs it.
TEST_SRCS=(
  "$CODE/tests/shared/TestUtils.cpp"
  "$CODE/tests/shared/TestFormData.cpp"
  "$CODE/tests/shared/TestNetBuffer.cpp"
  "$CODE/tests/shared/TestMessageBuffer.cpp"
  "$CODE/tests/shared/TestArrayUpdate.cpp"
  "$CODE/tests/shared/TestNetEvent.cpp"
  "$CODE/tests/shared/TestGameStateNAck.cpp"
  "$CODE/tests/shared/TestComponents.cpp"
  "$CODE/components/net-base/src/NetBuffer.cpp"
  "$CODE/components/net-base/src/FormData.cpp"
  "$SRC/mayhem/tests_main.cpp"
)
TEST_INC=("${INC[@]}" -I"$CODE/tests/shared" -I"$CODE/vendor/catch2/include")
i=0
pids=()
for f in "${TEST_SRCS[@]}"; do
  i=$((i+1))
  $CXX "${CXXSTD[@]}" -O1 $COVERAGE_FLAGS "${TEST_INC[@]}" -DCATCH_AMALGAMATED_CUSTOM_MAIN -c "$f" -o "$OUT/test/$i.o" &
  pids+=($!)
  if [ "${#pids[@]}" -ge "$MAYHEM_JOBS" ]; then wait "${pids[0]}"; pids=("${pids[@]:1}"); fi
done
$CXX -std=c++17 -O1 $COVERAGE_FLAGS -I"$CODE/vendor/catch2/include" -DCATCH_AMALGAMATED_CUSTOM_MAIN \
  -c "$CODE/vendor/catch2/src/catch_amalgamated.cpp" -o "$OUT/test/catch.o" &
pids+=($!)
for p in "${pids[@]}"; do wait "$p"; done
$CXX $COVERAGE_FLAGS "$OUT"/test/*.o -o "$OUT/netparse_tests"

# 2) Project code (net-base) + leak-sanitizer hook, with $SANITIZER_FLAGS and DWARF<=3. Every object is built
#    twice: instrumented for the fuzzing engine (fuzz/, compiled with $LIB_FUZZING_ENGINE so coverage
#    feedback reaches the code under test) and plain (standalone/, for the run-once reproducers).
FLAGS=(${SANITIZER_FLAGS} ${DEBUG_FLAGS} -O1)
mkdir -p "$OUT/standalone"
for f in NetBuffer FormData; do
  $CXX "${CXXSTD[@]}" "${FLAGS[@]}" $LIB_FUZZING_ENGINE "${INC[@]}" -c "$CODE/components/net-base/src/$f.cpp" -o "$OUT/fuzz/$f.o"
  $CXX "${CXXSTD[@]}" "${FLAGS[@]}" "${INC[@]}" -c "$CODE/components/net-base/src/$f.cpp" -o "$OUT/standalone/$f.o"
done
$CXX "${CXXSTD[@]}" "${FLAGS[@]}" -c "$SRC/mayhem/lsan_off.cc" -o "$OUT/fuzz/lsan_off.o"
$CC "${FLAGS[@]}" -c "$STANDALONE_FUZZ_MAIN" -o "$OUT/standalone/standalone_main.o"

# 3) Harnesses: libFuzzer binary + non-fuzzer run-once reproducer for each.
# RELAX holds per-harness `-fno-sanitize=<check>` flags (benign-pattern UBSan checks; ASan and every
# other UBSan check stay on and halting). See the comments at each call.
build_harness() {
  local name="$1"; shift
  local libs=("$@")
  $CXX "${CXXSTD[@]}" "${FLAGS[@]}" ${RELAX:-} $LIB_FUZZING_ENGINE "${INC[@]}" -c "$SRC/mayhem/$name.cpp" -o "$OUT/fuzz/$name.o"
  $CXX "${FLAGS[@]}" $LIB_FUZZING_ENGINE "$OUT/fuzz/$name.o" "${libs[@]/#/$OUT/fuzz/}" "$OUT/fuzz/lsan_off.o" -o "$OUT/$name"
  $CXX "${CXXSTD[@]}" "${FLAGS[@]}" ${RELAX:-} "${INC[@]}" -c "$SRC/mayhem/$name.cpp" -o "$OUT/standalone/$name.o"
  $CXX "${FLAGS[@]}" "$OUT/standalone/$name.o" "${libs[@]/#/$OUT/standalone/}" "$OUT/fuzz/lsan_off.o" "$OUT/standalone/standalone_main.o" -o "$OUT/$name-standalone"
}
# alignment: ByteReader::Field(Span<T>) reinterprets an arbitrary input offset as T* (benign on x86).
RELAX="-fno-sanitize=alignment" build_harness fuzz_client_packets
RELAX="-fno-sanitize=alignment" build_harness fuzz_server_packets
build_harness fuzz_form_data FormData.o
build_harness fuzz_net_buffer NetBuffer.o
# shift-base: CopyBits is decompiled game code that does `-1 << n` (arithmetic shift on clang, intended).
RELAX="-fno-sanitize=shift-base" build_harness fuzz_rl_message_buffer
